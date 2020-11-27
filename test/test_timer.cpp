#include <chrono>
#include <libcoro.h>

using namespace libcoro;

void ResumeAbleMainTimer() {
    using namespace std::chrono;
    auto th = ThisScheduler()->Timer()->AddHandler(system_clock::now() + 5s,
    [](bool bValue) {
       if (bValue) {
           printf("timer canceled.\n");
       } else {
           printf("timer after 5s.\n");
       }
    });
    auto th2 = ThisScheduler()->Timer()->AddHandler(1s,
        [&th](bool) {
          printf("timer after 1s.\n");
          th.Stop();
    });

    ThisScheduler()->RunUtilNoTask();
    th2.Stop();
}

int main() {
    ResumeAbleMainTimer();
    return 0;
}
