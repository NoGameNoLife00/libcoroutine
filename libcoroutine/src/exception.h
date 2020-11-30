#ifndef LIBCOROUTINE_EXCEPTION_H
#define LIBCOROUTINE_EXCEPTION_H

#include <stdexcept>

namespace libcoro {
    enum class ErrorCode {
        None,
        NotReady,
        TimerCanceled,
        NotAwaitLock,
        StopRequested,
        MaxCount,
    };

    const char* GetErrorString(ErrorCode e, const char* class_name);

    class FutureException : std::logic_error {
    public:
        ErrorCode err_;
        FutureException(ErrorCode e)
        : std::logic_error(GetErrorString(e, "FutureException")), err_(e) {
        }
    };

class CancellationException : public std::logic_error {
public:
    ErrorCode err_;
    CancellationException(ErrorCode e) :
    std::logic_error(GetErrorString(e, "CancellationException")) {
    }
};
}

#endif //LIBCOROUTINE_EXCEPTION_H
