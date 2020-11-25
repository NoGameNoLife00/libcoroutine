#include <libcoro.h>
#include <thread>

using namespace libcoro;

template <class Tp>
static void CallbackGetLong(int64_t val, Tp&& cb) {
    using namespace std::chrono;
    std::thread([val, cb = std::forward<Tp>(cb)] {
        std::this_thread::sleep_for(500ms);
        cb(val*val);
    }).detach();
}

static Future<int64_t> AsyncGetLong(int64_t val) {
    Awaitable<int64_t> awaitable;
    CallbackGetLong(val, [awaitable](int64_t val) {
        awaitable.SetValue(val);
    });
    return awaitable.GetFuture();
}

static Future<int64_t> WaitGetLong(int64_t val) {
    val = co_await AsyncGetLong(val);
    co_return val;
}

static Future<int64_t> ResumeAbleGetLong(int64_t val) {
    printf("%ld\n", val);
    val = co_await WaitGetLong(val);
    printf("%ld\n", val);
    val = co_await WaitGetLong(val);
    printf("%ld\n", val);
    val = co_await WaitGetLong(val);
    printf("%ld\n", val);
    co_return val;
}

static Future<int64_t> LoopGetLong(int64_t val) {
    printf("%ld\n", val);
    for (int i = 0; i < 5; i++) {
        val = co_await AsyncGetLong(val);
        printf("%ld\n", val);
    }
    co_return val;
}

static Future<std::string&> AsyncGetString(std::string& ref_string) {
    Awaitable<std::string&> awaitable;
    CallbackGetLong(std::stoi(ref_string), [awaitable, &ref_string](int64_t val) {
        ref_string = std::to_string(val);
        awaitable.SetValue(ref_string);
    });
    return awaitable.GetFuture();
}

static Future<std::string&> ResumeAbleGetString(std::string& val) {
    printf("%s\n", val.c_str());
    val = co_await AsyncGetString(val);
    printf("%s\n", val.c_str());
    val = co_await AsyncGetString(val);
    printf("%s\n", val.c_str());
    val = co_await AsyncGetString(val);
    printf("%s\n", val.c_str());
    co_return static_cast<std::string&>(val);
}

void ResumeAbleMainCb() {
    printf("%s\n", __FUNCTION__ );
    go AsyncGetLong(3);
    ThisScheduler()->RunUtilNoTask();

    std::string ref_string{"2"};
    go ResumeAbleGetString(ref_string);
    ThisScheduler()->RunUtilNoTask();
    go [=]()mutable->libcoro::Future<> {
        auto val = co_await ResumeAbleGetLong(2);
        printf("GO:%ld\n", val);
    };
    go LoopGetLong(3);
    ThisScheduler()->RunUtilNoTask();
}
int main()
{
    ResumeAbleMainCb();
    return 0;
}

