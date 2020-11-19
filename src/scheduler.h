#ifndef LIBCOROUTINE_SCHEDULER_H
#define LIBCOROUTINE_SCHEDULER_H
#include <define.h>
#include <use_ptr.h>
#include <vector>
#include <unordered_map>
#include <task.h>
#include <co_type_traits.h>
#include <spinlock.h>
#include <timer.h>

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
        requires(traits::IsCallableV<T> || traits::IsFutureV<T> || traits::IsGeneratorV<T>)
        Task* operator+(T&& coro) {
            if constexpr (traits::IsCallableV<T>)
                return NewTask(new TaskCtxImpl<T>(coro));
            else
                return NewTask(new TaskImpl<T>(coro));
        }

        bool Empty() const {
            scoped_lock<spinlock, spinlock> guard(lock_ready_, lock_running_);
            return ready_task_.empty() && running_states_.empty() && timer_->Empty();
        }

        TimerManager* Timer() const {
            return timer_.get();
        }

        void AddGenerator(StateBase *s);
        void DelFinal(StateBase *s);
        std::unique_ptr<Task> DelSwitch(StateBase *s);
        void AddSwitch(std::unique_ptr<Task> task);
        Task* FindTask(StateBase* s) const;
        friend class LocalScheduler;
        static Scheduler g_scheduler;
    protected:
        Scheduler();
    private:
        using StateBasePtr = use_ptr<StateBase>;
        using StateArray = std::vector<StateBasePtr>;
        using LockType = spinlock;
        using TaskDictionaryType = std::unordered_map<StateBase*, std::unique_ptr<Task>>;

        mutable spinlock lock_running_;

        StateArray running_states_;
        StateArray cached_states_;
        mutable spinlock lock_ready_;

        TaskDictionaryType ready_task_;

        TimerMgrPtr timer_;

        Task* NewTask(Task* task);

    };

    class LocalScheduler {
    public:
        LocalScheduler();

        LocalScheduler(Scheduler& sch);

        ~LocalScheduler();

        LocalScheduler(LocalScheduler&&) = delete;
        LocalScheduler(const LocalScheduler&) = delete;
        LocalScheduler& operator=(LocalScheduler&&) = delete;
        LocalScheduler& operator=(const LocalScheduler&) = delete;
    private:
        Scheduler* scheduler_;
    };

    inline void Scheduler::AddGenerator(StateBase *state) {
        assert(state != nullptr);
        scoped_lock<spinlock> guard(lock_running_);
        running_states_.emplace_back(state);
    }

    inline void Scheduler::DelFinal(StateBase *state) {
        scoped_lock<spinlock> guard(lock_ready_);
        ready_task_.erase(state);
    }

    inline void Scheduler::AddSwitch(std::unique_ptr<Task> task) {
        StateBase* state = task->state_.get();
        scoped_lock<spinlock> guard(lock_ready_);
        ready_task_.emplace(state, std::move(task));
    }

    inline Task* Scheduler::FindTask(StateBase* state) const {
        scoped_lock<spinlock> guard(lock_ready_);
        auto it = ready_task_.find(state);
        if (it != ready_task_.end()) {
            return it->second.get();
        }
        return nullptr;
    }
}




#endif //LIBCOROUTINE_SCHEDULER_H
