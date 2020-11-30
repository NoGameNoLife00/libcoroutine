#include <libcoro.h>
#include <thread>
#include <chrono>


using namespace libcoro;
static std::mutex print_mutex;

auto AsyncHeavyComputingTasks(int64_t val) {
    using namespace std::chrono;
    Awaitable<int64_t> awaitable;
    std::thread([val, st= awaitable.state_] {
        std::this_thread::sleep_for(500ms);
        st->SetValue(val*val);
    }).detach();
    return awaitable.GetFuture();
}

Future<> HeavyComputingSequential(int64_t val) {
    for (size_t i = 0; i < 3; i++) {
        {
            scoped_lock<std::mutex> lock(print_mutex);
            printf("%ld @ %ld\n", val, std::this_thread::get_id());
        }
        val = co_await AsyncHeavyComputingTasks(val);
    }

    {
        scoped_lock<std::mutex> lock(print_mutex);
        printf("%ld @ %ld\n", val, std::this_thread::get_id());
    }
}

void TestUseSingleThread(int64_t val) {
    LocalScheduler my_scheduler;

    {
        scoped_lock<std::mutex> lock(print_mutex);
        printf("running in thread @%ld\n", std::this_thread::get_id());
    }
    go HeavyComputingSequential(val);
    ThisScheduler()->RunUtilNoTask();
}

const size_t N = 2;
void TestUseMultiThread() {
    std::thread t_array[N];
    for (size_t i= 0; i < N; i++) {
        t_array[i] = std::thread(&TestUseSingleThread, 4+i);
    }
    TestUseSingleThread(3);
    for (auto& t: t_array) {
        t.join();
    }
}

void ResumeAbleMainMultiThread() {
    printf("TestUseSingleThread @%ld\n\n", std::this_thread::get_id());
    TestUseSingleThread(2);
    printf("\n");


    printf("TestUseMultiThread @%d\n\n", std::this_thread::get_id());
    TestUseMultiThread();

    Scheduler::g_scheduler.RunUtilNoTask();
}


int main()
{
    ResumeAbleMainMultiThread();
    return 0;
}


