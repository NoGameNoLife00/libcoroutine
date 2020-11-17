#ifndef LIBCOROUTINE_STATE_H
#define LIBCOROUTINE_STATE_H
#include <atomic>
#include <define.h>
#include <memory>
#include <cstddef>
#include <coroutine>
namespace libcoro {
    class StateBase {
    public:
        using AllocChar = std::allocator<char>;

        void lock() {
            count_.fetch_add(1, std::memory_order_acq_rel);
        }
        void unlock() {
            if (unlikely(count_.fetch_sub(1, std::memory_order_acq_rel) == 1)) {
                DestroyDeallocate();
            }
        }
        virtual void Resume() = 0;
        virtual bool HasHandler() const = 0;
        virtual StateBase* GetParent() const;

        void SetScheduler(Scheduler* sch) {
            scheduler_ = sch;
        }

        coroutine_handle<> GetHandler() const {
            return coro_;
        }

        StateBase* GetRoot() const {
            StateBase* root = const_cast<StateBase*>(this);
            StateBase* next = root->GetParent();
            while (next != nullptr) {
                root = next;
                next = root->GetParent();
            }
            return root;
        }

        Scheduler* GetScheduler() const {
            return GetRoot()->scheduler_;
        }
    protected:
        Scheduler* scheduler_ = nullptr;
        coroutine_handle<> coro_;
        virtual ~StateBase();
    private:
        virtual void DestroyDeallocate();
        std::atomic<intptr_t> count_{0};
    };

    class StateGenerator : public StateBase {
    public:
        virtual void Resume() override;
        virtual bool HasHandler() const override;

        void SetInitialSuspend(coroutine_handle<> handler) {
            coro_ = handler;
        }

        static StateGenerator* Construct(void* ptr) {
            return new(ptr) StateGenerator();
        }

        static StateGenerator* AllocState();

    private:
        StateGenerator() = default;
        virtual void DestroyDeallocate() override;

    };
}




#endif //LIBCOROUTINE_STATE_H
