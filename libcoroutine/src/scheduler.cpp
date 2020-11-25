#include <libcoro.h>

#if LIBCORO_DEBUG
std::mutex g_coro_cout_mutex;
std::atomic<intptr_t> g_coro_state_count = 0;
std::atomic<intptr_t> g_coro_task_count = 0;
std::atomic<intptr_t> g_coro_evtctx_count = 0;
std::atomic<intptr_t> g_coro_state_id = 0;
#endif

namespace libcoro {
    thread_local Scheduler* t_scheduler = nullptr;
    Scheduler Scheduler::g_scheduler;
    Scheduler* ThisScheduler() {
        return t_scheduler ? t_scheduler : &Scheduler::g_scheduler;
    }


    const char * future_error_string[static_cast<size_t>(ErrorCode::MaxCount)] {
      "none",
      "not_ready",
      "timer_canceled",
      "not_await_lock",
      "stop_requested",
    };

    char sz_future_error_buffer[256];
    const char *GetErrorString(ErrorCode e, const char *class_name) {
        if (class_name) {
            sprintf(sz_future_error_buffer, "%s, code=%s",
                    class_name, future_error_string[static_cast<size_t>(e)]);
            return sz_future_error_buffer;
        }
        return future_error_string[static_cast<size_t>(e)];
    }




    Task *Scheduler::NewTask(Task *task) {
        StateBase* state = task->state_.get();
        state->SetScheduler(this);
        {
            scoped_lock<spinlock> guard(lock_ready_);
            ready_task_.emplace(state, task);
        }
        if (state->HasHandler()) {
            AddGenerator(state);
        }
        return task;
    }

    Scheduler::~Scheduler() {
        if (t_scheduler == this)
            t_scheduler = nullptr;
    }

    std::unique_ptr<Task> Scheduler::DelSwitch(StateBase *state) {
        scoped_lock<spinlock> guard(lock_ready_);
        std::unique_ptr<Task> task;
        auto it = this->ready_task_.find(state);
        if (it != this->ready_task_.end()) {
            task = std::exchange(it->second, nullptr);
            this->ready_task_.erase(it);
        }
        return task;
    }

    bool Scheduler::RunOneBatch() {
        this->timer_->Update();
        {
            scoped_lock<spinlock> guard(lock_running_);
            if (likely(running_states_.empty())) {
                return false;
            }
            std::swap(cached_states_, running_states_);
        }

        for (StateBasePtr& state : cached_states_) {
            state->Resume();
        }
        cached_states_.clear();
        return true;
    }

    void Scheduler::RunUtilNoTask() {
        for(;;) {
            if (likely(this->RunOneBatch()))
                continue;

            {
                scoped_lock<spinlock> guard(lock_ready_);
                if (likely(!ready_task_.empty()))
                    continue;

            }

            if (unlikely(!timer_->Empty()))
                continue;

            break;
        }
    }

    Scheduler::Scheduler() : timer_(std::make_shared<TimerManager>()) {
        running_states_.reserve(1024);
        cached_states_.reserve(1024);
        if (t_scheduler == nullptr) {
            t_scheduler = this;
        }
    }

    LocalScheduler::LocalScheduler() {
        if (t_scheduler == nullptr) {
            scheduler_ = new Scheduler;
            t_scheduler = scheduler_;
        } else {
            scheduler_ = nullptr;
        }
    }

    LocalScheduler::LocalScheduler(Scheduler &sch) {
        if (t_scheduler == nullptr) {
            t_scheduler = &sch;
        }
        t_scheduler = nullptr;
    }

    LocalScheduler::~LocalScheduler() {
        if (t_scheduler == scheduler_) {
            t_scheduler = nullptr;
        }
        delete scheduler_;
    }
}

