#include <libcoro.h>

namespace libcoro {
    Future<> SleepUntil_(std::chrono::system_clock::time_point tp, Scheduler &scheduler) {
        Awaitable<> awaitable;
        (void)scheduler.Timer()->Add(tp,
            [awaitable](bool cancellation_requested) {
                if (cancellation_requested) {
                    awaitable.ThrowException(CancellationException{ErrorCode::TimerCanceled});
                } else {
                    awaitable.SetValue();
                }
        });
        return awaitable.GetFuture();
    }

}
