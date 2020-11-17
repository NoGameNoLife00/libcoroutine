#ifndef LIBCOROUTINE_TIMER_H
#define LIBCOROUTINE_TIMER_H
#include <define.h>
namespace libcoro {
    class TimerManager;

    typedef std::shared_ptr<TimerManager> TimerMgrPtr;
    typedef std::weak_ptr<TimerManager> TimerMgrWPtr;
}



#endif //LIBCOROUTINE_TIMER_H
