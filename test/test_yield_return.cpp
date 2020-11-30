#include <libcoro.h>
#include <thread>
#include <chrono>

using namespace libcoro;

Generator<int> TestYieldInt() {
    printf("1 will yield return\n");
    co_yield 1;
    printf("2 will yield return\n");
    co_yield 2;
    printf("3 will yield return\n");
    co_yield 3;
    printf("4 will return\n");
    co_return 4;

    printf("5 will never yield return\n");
    co_yield 5;
}

auto TestYieldVoid() -> Generator<> {
    printf("block 1 will yield return\n");
    co_yield nullptr;
    printf("block 2 will yield return\n");
    co_yield nullptr;
    printf("block 3 will yield return\n");
    co_yield nullptr;
    printf("block 4 will  return\n");
    co_return nullptr;

    printf("block 5 will never yield return\n");
    co_yield nullptr;
}

auto TestYieldFuture() ->Future<int64_t> {
    printf("future 1 will yield return\n");
    co_yield 1;
    printf("future 2 will yield return\n");
    co_yield 2;
    printf("future 3 will yield return\n");
    co_yield 3;
    printf("future 4 will return\n");
    co_return 4;

    printf("future 5 will never yield return\n");
    co_yield 5;
}

void ResumeAbleMainYieldReturn() {
    printf("%s\n", __FUNCTION__ );
    for (int i : TestYieldInt()) {
        printf("%d had return\n", i);
    }

    go TestYieldInt();
    ThisScheduler()->RunUtilNoTask();

    go TestYieldVoid();
    ThisScheduler()->RunUtilNoTask();

    go TestYieldFuture();
    ThisScheduler()->RunUtilNoTask();
}
int main()
{
    ResumeAbleMainYieldReturn();
    return 0;
}