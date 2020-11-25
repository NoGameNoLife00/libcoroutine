
#ifndef LIBCOROUTINE_TASK_H
#define LIBCOROUTINE_TASK_H

#include <libcoro.h>

namespace libcoro {
    class Task {
    public:
        Task() : stop_(nostopstate) {}

        virtual ~Task() {}

        const stop_source& GetStopSource() {
            return stop_;
        }

        stop_token GetStopToken() {
            return GetStopSource().get_token();
        }

        bool RequestStop() {
            return GetStopSource().request_stop();
        }

        bool RequestStopIfPossible() const {
            if (stop_.stop_possible()) {
                return stop_.stop_requested();
            }
            return false;
        }

    protected:
        friend Scheduler;
        use_ptr<StateBase> state_;
        stop_source stop_;
    };

    template<class Tp, typename = std::void_t<>>
    class TaskImpl;

    template<class Tp>
    class TaskImpl<Tp, std::void_t<traits::IsFuture<std::remove_reference_t<Tp>>>> : public Task {
    public:
        using FutureType = std::remove_reference_t<Tp>;
        using ValueType = typename FutureType::ValueType;
        using StateType = State<ValueType>;

        TaskImpl() = default;
        TaskImpl(FutureType& f) {
            Initialize(f);
        }

    protected:
        void Initialize(FutureType& f) {
            state_ = f.state_.get();
        }
    };

    template<class Tp>
    class TaskImpl<Generator<Tp>> : public Task {
    public:
        using ValueType = Tp;
        using FutureType = Generator<ValueType>;
        using StateType = StateGenerator;

        TaskImpl() = default;
        TaskImpl(FutureType& f) {
            Initialize(f);
        }

    protected:
        void Initialize(FutureType& f) {
            state_ = f.DetachState();
        }
    };

    template<class Ctx>
    class TaskCtxImpl : public TaskImpl<RemoveCvRefT<decltype(std::declval<Ctx>()())>> {
    public:
        using ContextType = Ctx;
        ContextType context_;

        TaskCtxImpl(ContextType ctx) : context_(std::move(ctx)) {
            decltype(auto) f = context_();
            this->Initialize(f);
        }
    };
}



#endif //LIBCOROUTINE_TASK_H
