#include <libcoro.h>
#include "timer.h"

namespace libcoro {

    TimerManager::TimerManager() {
        added_timers_.reserve(128);
    }

    TimerManager::~TimerManager() {
        Clear();
    }

    bool TimerManager::Stop(const TimerManager::TimerTargetPtr &tt) {
        if (!tt || tt->st_ == TimerTarget::State::Invalid) {
            return false;
        }
#ifdef _DEBUG
        assert(tt->manager_ == this);
#endif
        tt->st_ = TimerTarget::State::Invalid;
        return true;
    }

    void TimerManager::Update() {
        {
            std::unique_lock<spinlock> lock(added_mtx_);
            if (unlikely(!added_timers_.empty())) {
                auto add_timers = std::move(added_timers_);
                added_timers_.reserve(128);
                lock.unlock();

                for(auto& timer : add_timers) {
                    if (timer->st_ == TimerTarget::State::Added) {
                        timer->st_ = TimerTarget::State::Running;
                        running_timers_.emplace(timer->tp_, timer);
                    } else {
                        assert(timer->st_ == TimerTarget::State::Invalid);
                        CallTarget_(timer, true);
                    }
                }
            }
            if (unlikely(!running_timers_.empty())) {
                auto now = ClockType::now();
                auto it = running_timers_.begin();
                for (; it != running_timers_.end(); it++) {
                    if (it->first > now) {
                        break;
                    }
                    CallTarget_(it->second, it->second->st_ == TimerTarget::State::Invalid);
                }
                running_timers_.erase(running_timers_.begin(), it);
            }
        }
    }

    void TimerManager::CallTarget_(const TimerManager::TimerTargetPtr &tt, bool cancel) {
        auto cb = std::move(tt->cb_);
        tt->st_ = TimerTarget::State::Invalid;
#ifdef _DEBUG
        tt->manager_ = nullptr;
#endif
        if (cb) {
            cb(cancel);
        }
    }

    void TimerManager::Clear() {
        std::unique_lock<spinlock> lock(added_mtx_);
        auto add_timers = std::move(added_timers_);
        lock.unlock();

        for (auto& t : add_timers) {
            CallTarget_(t, true);
        }
        auto r_timers = std::move(running_timers_);
        for (auto& kv : r_timers) {
            CallTarget_(kv.second, true);
        }
    }

    TimerManager::TimerTargetPtr TimerManager::Add_(const TimerManager::TimerTargetPtr &tt) {
        assert(tt);
        assert(tt->st_ == TimerTarget::State::Invalid);

        std::scoped_lock<spinlock> lock(added_mtx_);
#ifdef _DEBUG
        assert(tt->manager_ == nullptr);
        tt->manager_ = this;
#endif
        tt->st_ = TimerTarget::State::Added;
        added_timers_.push_back(tt);
        return tt;
    }

}