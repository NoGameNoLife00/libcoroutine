#include <chrono>
#include <libcoro.h>
#include <string>


using namespace libcoro;

Future<> TestSleepUseTimer() {
    using namespace std::chrono;

    (void)SleepFor(100ms);
    co_await SleepFor(100ms);
    printf("sleep for 100ms.\n");
    co_await 100ms;
    printf("co_await 100ms.\n");



    try {
        co_await SleepUntil(system_clock::now() + 200ms);
        printf("timer after 200ms.\n");
    } catch (CancellationException) {
        printf("timer canceled.\n");
    }
}
void ResumeAbleMainSleep() {
    go TestSleepUseTimer();
    ThisScheduler()->RunUtilNoTask();
    printf("\n");
}

int main() {
    ResumeAbleMainSleep();
    return 0;
}