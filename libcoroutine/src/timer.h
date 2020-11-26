#ifndef LIBCOROUTINE_TIMER_H
#define LIBCOROUTINE_TIMER_H
#include <libcoro.h>

namespace libcoro {
    class TimerManager;

    typedef std::shared_ptr<TimerManager> TimerMgrPtr;
    typedef std::weak_ptr<TimerManager> TimerMgrWPtr;

    namespace detail {
        using TimerClockType = std::chrono::system_clock;
        using TimerCallbackType = std::function<void(bool)>;

        class TimerTarget : public std::enable_shared_from_this<TimerTarget> {
        public:
            TimerTarget(const TimerClockType::time_point& tp, const TimerCallbackType& cb)
                : tp_(tp), cb_(cb){}
            TimerTarget(const TimerClockType::time_point& tp, TimerCallbackType && cb)
                : tp_(tp), cb_(std::forward<TimerCallbackType>(cb)) {}
            friend TimerManager;
        private:
            enum class State : uint32_t {
                Invalid,
                Added,
                Running,
            };
            TimerClockType::time_point tp_;
            TimerCallbackType cb_;
            State st_ = State::Invalid;
#ifdef _DEBUG
            TimerManager* manager_ = nullptr;
#endif
        };

        typedef std::shared_ptr<TimerTarget> TimerTargetPtr;
        typedef std::weak_ptr<TimerTarget> TimerTargetWPtr;
    }

    class TimerHandler {
    public:
        TimerHandler() = default;
        TimerHandler(const TimerHandler&) = default;
        TimerHandler& operator=(const TimerHandler&) = default;
        inline TimerHandler(TimerHandler && r)
        : manager_(std::move(r.manager_)), target_(std::move(r.target_))  {}

        inline TimerHandler& operator=(TimerHandler&& r) {
            if (this != &r) {
                manager_ = std::move(r.manager_);
                target_ = std::move(r.target_);
            }
            return *this;
        }
        TimerHandler(TimerManager* manager, const detail::TimerTargetPtr& target);

        void Reset();
        bool Stop();
        bool Expired() const;
    private:
        TimerMgrWPtr manager_;
        detail::TimerTargetWPtr target_;
    };

    class TimerManager : public std::enable_shared_from_this<TimerManager> {
    public:
        using TimerTarget = detail::TimerTarget;
        using TimerTargetPtr = detail::TimerTargetPtr;
        using ClockType = detail::TimerClockType;
        using DurationType = ClockType::duration;
        using TimePointType = ClockType::time_point;
        using TimerArrayType = std::vector<TimerTargetPtr>;
        using TimerMapType = std::multimap<ClockType::time_point, TimerTargetPtr>;

        TimerManager();
        ~TimerManager();

        template<class Rep, class Period, class Cb>
        TimerTargetPtr Add(const std::chrono::duration<Rep, Period>& dt, Cb cb) {
            return Add_(std::chrono::duration_cast<DurationType>(dt), std::forward<Cb>(cb));
        }

        template<class Clock, class Duration = typename Clock::duration, class Cb>
        TimerTargetPtr Add(const std::chrono::time_point<Clock, Duration>& tp, Cb cb) {
            return Add_(std::chrono::time_point_cast<DurationType>(tp), std::forward<Cb>(cb));
        }

        template<class Rep, class Period, class Cb>
        TimerHandler AddHandler(const std::chrono::duration<Rep, Period>& dt, Cb& cb) {
            return {this, Add(dt, std::forward<Cb>(cb))};
        }

        template<class Clock, class Duration = typename Clock::duration, class Cb>
        TimerHandler AddHandler(const std::chrono::time_point<Clock, Duration>& tp, Cb&& cb) {
            return {this, Add(tp, std::forward<Cb>(cb))};
        }

        bool Stop(const TimerTargetPtr& tt);

        inline bool Empty() const {
            return running_timers_.empty() && added_timers_.empty();
        }
        void Clear();
        void Update();


    private:
        spinlock added_mtx_;
        TimerArrayType  added_timers_;
        TimerMapType running_timers_;

        template<class Cb>
        TimerTargetPtr Add_(const DurationType& dt, Cb&& cb) {
            return Add_(std::make_shared<TimerTarget>(ClockType::now() + dt, std::forward<Cb>(cb)));
        }

        template<class Cb>
        TimerTarget Add_(const TimePointType& tp, Cb&& cb) {
            return Add_(std::make_shared<TimerTarget>(tp, std::forward<Cb>(cb)));
        }
        TimerTargetPtr Add_(const TimerTargetPtr& tt);
        static void CallTarget_(const TimerTargetPtr& tt, bool cancel);
    };


    //--------------------

    inline TimerHandler::TimerHandler(TimerManager* manager, const detail::TimerTargetPtr& target)
    : manager_(manager->shared_from_this()), target_(target) {
    }



    inline void TimerHandler::Reset() {
        manager_.reset();
        target_.reset();
    }

    inline bool TimerHandler::Expired() const {
        return target_.expired();
    }

    inline bool TimerHandler::Stop() {
        bool result = false;
        if (!target_.expired()) {
            auto ptr = manager_.lock();
            if (ptr) {
                result = ptr->Stop(target_.lock());
            }
            target_.reset();
        }
        return result;
    }
}



#endif //LIBCOROUTINE_TIMER_H
