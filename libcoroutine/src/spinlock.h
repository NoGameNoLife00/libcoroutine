#ifndef LIBCOROUTINE_SPINLOCK_H
#define LIBCOROUTINE_SPINLOCK_H

#include <libcoro.h>

namespace libcoro {
    class spinlock {
    public:
        static const size_t MAX_ACTIVE_SPIN = 4000;
        static const size_t MAX_YIELD_SPIN = 8000;
        static const int FREE_VALUE = 0;
        static const int LOCKED_VALUE = 1;

        std::atomic_int lck;
//#ifdef LIBCORO_DEBUG
        std::thread::id owner_thread_id;
//#endif

        spinlock() {
            lck = FREE_VALUE;
        }

        void lock() {
            int val = FREE_VALUE;
            if (!lck.compare_exchange_weak(val, LOCKED_VALUE, std::memory_order_acq_rel)) {
//#ifdef LIBCORO_DEBUG
                assert(owner_thread_id != std::this_thread::get_id());
//#endif
                size_t spin_count = 0;
                auto dt = std::chrono::milliseconds{1};
                do {
                    while (lck.load(std::memory_order_relaxed) != FREE_VALUE) {
                        if (spin_count < MAX_ACTIVE_SPIN) {
                            ++spin_count;
                        } else if (spin_count < MAX_YIELD_SPIN) {
                            ++ spin_count;
                            std::this_thread::yield();
                        } else {
                            std::this_thread::sleep_for(dt);
                            dt = std::min(std::chrono::milliseconds{128}, dt * 2);
                        }
                    }
                    val = FREE_VALUE;
                } while (!lck.compare_exchange_weak(val, LOCKED_VALUE, std::memory_order_acq_rel));
            }
#ifdef LIBCORO_DEBUG
            owner_thread_id = std::this_thread::get_id();
#endif
        }

        bool try_lock() {
            int val = FREE_VALUE;
            bool ret = lck.compare_exchange_strong(val, LOCKED_VALUE, std::memory_order_acq_rel);
#ifdef LIBCORO_DEBUG
            if (ret) {
                owner_thread_id = std::this_thread::get_id();
            }
#endif
            return ret;
        }

        void unlock() {
#if LIBCORO_DEBUG
            owner_thread_id = std::thread::id();
#endif
            lck.store(FREE_VALUE, std::memory_order_release);
        }
    };
}
#endif //LIBCOROUTINE_SPINLOCK_H
