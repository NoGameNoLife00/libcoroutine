#include <chrono>
#include <libcoro.h>
#include <thread>

using namespace libcoro;


auto AsyncSignalException(const intptr_t dividend) {
    Awaitable<int64_t> awaitable;
    std::thread([dividend, awaitable] {
       std::this_thread::sleep_for(std::chrono::milliseconds(50));
        try {
            if (dividend == 0) {
                throw std::logic_error("dividend by 0\n");
            }
            awaitable.SetValue(10000/dividend);
        } catch (...) {
            awaitable.SetException(std::current_exception());
        }
    }).detach();
    return awaitable.GetFuture();
}

auto AsyncSignalException2(const intptr_t dividend) {
    Awaitable<int64_t> awaitable;
    std::thread([dividend, awaitable] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        try {
            if (dividend == 0) {
                throw std::logic_error("dividend by 0\n");
            }
            awaitable.SetValue(10000/dividend);
        } catch (...) {
            awaitable.SetException(std::current_exception());
        }
    }).detach();
    return awaitable.GetFuture();
}

Future<> TestSignalException() {
    for (intptr_t i = 10; i >= 0; i--) {
        try {
            auto r = co_await AsyncSignalException2(i);
            printf("result is %d\n", r);
        } catch (const std::exception& ex) {
            printf("exception signal: %s\n", ex.what());
        } catch (...) {
            printf("exception signal: unknow\n");
        }
    }
}

Future<> TestBombException() {
    for (intptr_t i = 10; i >= 0; i--) {
        auto r = co_await AsyncSignalException(i);
        printf("result is %d\n", r);
    }
}

void ResumeAbleMainException(bool bomb) {
    printf("%s\n", __FUNCTION__ );
    go TestBombException();
    ThisScheduler()->RunUtilNoTask();

    printf("\n");
    if (bomb) {
        go TestBombException();
        ThisScheduler()->RunUtilNoTask();
    }
}

int main() {
    ResumeAbleMainException(true);
    return 0;
}
