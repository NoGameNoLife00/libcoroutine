#include "scheduler.h"


#if LIBCORO_DEBUG
std::mutex g_coro_cout_mutex;
std::atomic<intptr_t> g_coro_state_count = 0;
std::atomic<intptr_t> g_coro_task_count = 0;
std::atomic<intptr_t> g_coro_evtctx_count = 0;
std::atomic<intptr_t> g_coro_state_id = 0;
#endif

namespace libcoro {
    bool Scheduler::RunOneBatch() {

    }
}

