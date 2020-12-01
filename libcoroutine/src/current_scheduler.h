#ifndef LIBCOROUTINE_CURRENT_SCHEDULER_H
#define LIBCOROUTINE_CURRENT_SCHEDULER_H
namespace libcoro {
    class GetCurrentSchedulerAwaiter {
    public:
        bool await_ready() const noexcept {
            return false;
        }

        template<class PromiseT, typename = std::enable_if_t<traits::IsPromiseV<PromiseT>>>
        bool await_suspend(coroutine_handle<PromiseT> handler) {
            PromiseT& promise = handler.promise();
            auto* state = promise.GetState();
            this->scheduler_ = state->GetScheduler();
            return false;
        }

        Scheduler* await_resume() const noexcept {
            return scheduler_;
        }

    private:
        Scheduler* scheduler_;
    };

    inline GetCurrentSchedulerAwaiter GetCurrentScheduler() noexcept {
        return {};
    }

    class GetRootStateAwaiter {
    public:
        bool await_ready() const noexcept {
            return false;
        }

        template<class PromiseT, typename = std::enable_if_t<traits::IsPromiseV<PromiseT>>>
        bool await_suspend(coroutine_handle<PromiseT> handler) {
            PromiseT& promise = handler.promise();
            auto* parent = promise.GetState();
            this->state_ = parent->GetRoot();
            return false;
        }

        StateBase* await_resume() const noexcept {
            return state_;
        }
    private:
        StateBase* state_;
    };

    inline GetRootStateAwaiter GetRootState() noexcept {
        return {};
    }

    class GetCurrentTaskAwaiter {
    public:
        bool await_ready() const noexcept {
            return false;
        }

        template<class PromiseT, typename = std::enable_if_t<traits::IsPromiseV<PromiseT>>>
        bool await_suspend(coroutine_handle<PromiseT> handler) {
            PromiseT& promise = handler.promise();
            auto* parent = promise.GetState();
            StateBase* state = parent->GetRoot();
            Scheduler* sch = state->GetScheduler();
            task_ = sch->FindTask(state);
            return false;
        }

        Task* await_resume() const noexcept {
            return task_;
        }
    private:
        Task* task_;
    };

    inline GetCurrentTaskAwaiter GetCurrentTask() noexcept {
        return {};
    }

}
#endif //LIBCOROUTINE_CURRENT_SCHEDULER_H
