#ifndef LIBCOROUTINE_SLEEP_H
#define LIBCOROUTINE_SLEEP_H

#include <libcoro.h>
namespace libcoro {

    Future<> SleepUntil_(std::chrono::system_clock::time_point tp, Scheduler& scheduler);

    template<class Clock, class Duration = typename Clock::duration>
    inline Future<> SleepUntil(std::chrono::time_point<Clock, Duration> tp, Scheduler& scheduler) {
        return SleepUntil_(std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp), scheduler);
    }

    inline Future<> SleepFor_(std::chrono::system_clock::duration dt, Scheduler& scheduler) {
        return SleepUntil_(std::chrono::system_clock::now() + dt, scheduler);
    }

    template<class Rep, class Period>
    inline Future<> SleepFor(std::chrono::duration<Rep, Period> dt, Scheduler& scheduler) {
        return SleepFor_(std::chrono::duration_cast<std::chrono::system_clock::duration>(dt), scheduler);
    }

    template<class Rep, class Period>
    inline Future<> SleepFor(std::chrono::duration<Rep, Period> dt) {
        Scheduler* sch = CurrentScheduler();
        co_await SleepFor_(std::chrono::duration_cast<std::chrono::system_clock::duration>(dt), *sch);
    }

    template<class Clock, class Duration>
    inline Future<> SleepUntil(std::chrono::time_point<Clock, Duration> tp) {
        Scheduler* sch = CurrentScheduler();
        co_await SleepUntil_(std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp), *sch);
    }

    template<class Rep, class Period>
    inline Future<> operator co_await(std::chrono::duration<Rep, Period> dt) {
        Scheduler* sch = CurrentScheduler();
        co_await SleepFor(dt, *sch);
    }
}



#endif //LIBCOROUTINE_SLEEP_H
