#include <chrono>
#include <thread>
#include <libcoro.h>

using namespace libcoro;
using namespace std::chrono;

template<class Ctype, typename = std::enable_if_t<std::is_invocable_v<Ctype, bool, int64_t>>>
static void CallbackGetLongWithStop(stop_token token, int64_t val, Ctype&& cb) {
    std::thread([val, token=std::move(token), cb=std::forward<Ctype>(cb)] {
        for(int i = 0; i < 10; i++) {
            if (token.stop_requested()) {
                cb(false, 0);
                return;
            }
            std::this_thread::sleep_for(10ms);
        }
        cb(true, val*val);
    }).detach();
}


static Future<int64_t> AsyncGetLongWithStop(stop_token token, int64_t val) {
    Awaitable<int64_t> awaitable;
    CallbackGetLongWithStop(token, val, [awaitable](bool ok, int64_t val) {
        if (ok) {
            awaitable.SetValue(val);
        } else {
            awaitable.ThrowException(CancellationException{ErrorCode::StopRequested});
        }
    });
    return awaitable.GetFuture();
}

static Future<int64_t> AsyncGetLongWithStop(int64_t val) {
    Task* task = CurrentTask();
    co_return co_await AsyncGetLongWithStop(task->GetStopToken(), val);
}

static void TestGetLongWithStop(int64_t val) {
    Task* task = GO {
        try {
            int64_t result = co_await AsyncGetLongWithStop(val);
            printf("%ld\n", result);
        } catch (const std::logic_error& e) {
            printf("%s\n", e.what());
        }
    };

    stop_source stops = task->GetStopSource();
    GO {
        co_await SleepFor(1ms*(rand()%300));
        stops.request_stop();
    };

    ThisScheduler()->RunUtilNoTask();
}

void ResumeAbleMainStopToken() {
    srand(time(nullptr));
    for (int i = 0; i < 10; i++) {
        TestGetLongWithStop(i);
    }
    printf("OK - stop_token\n");
}

int main() {
    ResumeAbleMainStopToken();
    return 0;
}