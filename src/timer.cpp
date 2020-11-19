#include <cassert>
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
#if _DEBUG
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

}