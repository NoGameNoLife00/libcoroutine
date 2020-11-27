#include <libcoro.h>

using namespace libcoro;

//Future<> TestRoutineUseTimer() {
//    for(size_t i = 0; i < 3; i++) {
//        co_await
//    }
//}

void ResumeAbleMainRoutine() {
    printf("%s\n", __FUNCTION__ );
    ThisScheduler()->RunUtilNoTask();
}

int main() {
    ResumeAbleMainRoutine();
}