#ifndef LIBCOROUTINE_SCHEDULER_H
#define LIBCOROUTINE_SCHEDULER_H
#include <define.h>
#include <use_ptr.h>
#include <vector>
#include <unordered_map>
#include <task.h>
#include <co_type_traits.h>

namespace libcoro {
    class Scheduler : public std::enable_shared_from_this<Scheduler> {
    public:
        ~Scheduler();
        Scheduler(Scheduler&&) = delete;
        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(Scheduler&&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;

        bool RunOneBatch();
        void RunUtilNoTask();

        template<typename T>
        requires(traits::IsCallableV(T) || traits::IsFutureV(T) || traits::IsGeneratorV(T))
        Task* operator+(T&& coro) {
            if constexpr (traits::is_callable_v(T))
                return NewTask(new TaskCtxImpl<T>(coro));
            else
                return NewTask(new TaskImpl<T>(coro));
        }

        bool Empty() const {
            scope_lock<SpinLock, SpinLock> guard(lock_ready_, lock_running_);
            return ready_task_.Empty() && running_states_.Empty() && timer_.Empty();
        }

        TimerManager* Timer() const {
            return timer_.get();
        }

        void AddGenerator(StateBase *sb);
        void DelGenerator(StateBase *sb);
        std::unique_ptr<Task> DelSwitch(StateBase *sb);
        void AddSwitch(std::unique_ptr<Task> task);
        Task* FindTask(StateBase* sb) const;
        friend class LockScheduler;
        static Scheduler g_scheduler;
    protected:
        Scheduler();
    private:
        using StateBasePtr = UsePtr<StateBase>;
        using StateArray = std::vector<StateBasePtr>;
        using LockType = SpinLock;
        using TaskDictionaryType = std::unordered_map<StateBase*, std::unique_ptr<Task>>;

        mutable SpinLock lock_running_;

        StateArray running_states_;
        StateArray cached_states_;
        mutable SpinLock lock_ready_;

        TaskDictionaryType ready_task_;

        TimerMgrPtr timer_;

        Task* NewTask(Task* task);

    };
}




#endif //LIBCOROUTINE_SCHEDULER_H
