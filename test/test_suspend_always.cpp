#include <chrono>
#include <libcoro.h>
#include <thread>

using namespace libcoro;

Future<> TestLoopSleep(size_t N, const char* ch) {
    using namespace std::chrono;
    for (size_t i = 0; i < N; i++) {
        co_await SleepFor(100ms);
        printf("%s", ch);
    }
    printf("\n");
}

Future<> TestRecursiveAwait() {
    printf("A:---1\n");
    co_await TestLoopSleep(5, "=");
    printf("A:---2\n");
    co_await TestLoopSleep(6, "=");
    printf("A:---3\n");
    co_await TestLoopSleep(7, "=");
    printf("A:---4\n");
}

Future<> TestRecursiveGo() {
    printf("B:---1\n");
    co_await TestLoopSleep(3, "+");
    printf("B:---2\n");
    go TestLoopSleep(8, "*");
    printf("B:---3\n");
    co_await TestLoopSleep(4, "+");
    printf("B:---4\n");
}

void ResumeAbleMainSuspendAlways() {
    printf("%s\n", __FUNCTION__ );
    go TestRecursiveAwait();
    go TestRecursiveGo();
    ThisScheduler()->RunUtilNoTask();
}

int main() {
    ResumeAbleMainSuspendAlways();
    return 0;
}