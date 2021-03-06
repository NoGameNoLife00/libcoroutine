#ifndef LIBCOROUTINE_SWITCH_SCHEDULER_H
#define LIBCOROUTINE_SWITCH_SCHEDULER_H
#include <libcoro.h>
namespace libcoro {
    class SwitchSchedulerAwaiter {
    public:
        using ValueType = void;
        using StateType = State<ValueType>;
        using promise_type = Promise<ValueType>;
        using LockType = typename StateType::LockType;

        SwitchSchedulerAwaiter(Scheduler* sch) : scheduler_(sch) {}
        SwitchSchedulerAwaiter(const SwitchSchedulerAwaiter&) = default;
        SwitchSchedulerAwaiter(SwitchSchedulerAwaiter&&) = default;

        SwitchSchedulerAwaiter& operator=(const SwitchSchedulerAwaiter&) = default;
        SwitchSchedulerAwaiter& operator=(SwitchSchedulerAwaiter&&) = default;

        bool await_read() const {
            return false;
        }

        template<PromiseT PromiseTp>
        bool await_suspend(coroutine_handle<PromiseTp> handler) {
            PromiseTp& promise = handler.Promise();
            auto* ptr = promise.GetState();
            if (ptr->SwitchSchedulerAwaitSuspend(scheduler_)) {
                use_ptr<State<void>> state = StateFuture::AllocState<StateType>(true);
                state->SetValue();
                state->FutureAwaitResume();
                return true;
            }
            return false;
        }

        void await_resume() const {
        }

    private:
        Scheduler* scheduler_;
    };

    inline SwitchSchedulerAwaiter Via(Scheduler& sch) {
        return {&sch};
    }

    inline SwitchSchedulerAwaiter Via(Scheduler* sch) {
        return {sch};
    }

}

#endif //LIBCOROUTINE_SWITCH_SCHEDULER_H
