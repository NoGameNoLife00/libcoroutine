#ifndef LIBCOROUTINE_STATE_H
#define LIBCOROUTINE_STATE_H
#include <atomic>
#include <define.h>
#include <memory>
#include <cstddef>
#include <coroutine>
#include <spinlock.h>
#include <co_type_traits.h>

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
        virtual StateBase* GetParent() const {
            return nullptr;
        }

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
        virtual ~StateBase() {}
    private:
        virtual void DestroyDeallocate();
        std::atomic<intptr_t> count_{0};
    };

    class StateGenerator : public StateBase {
    public:
        void Resume() override;
        bool HasHandler() const override;

        void SetInitialSuspend(coroutine_handle<> handler) {
            coro_ = handler;
        }

        static StateGenerator* Construct(void* ptr) {
            return new(ptr) StateGenerator();
        }

        static StateGenerator* AllocState();
        bool SwitchSchedulerAwaitSuspend(Scheduler* sch);
    private:
        StateGenerator() = default;
        void DestroyDeallocate() override;

    };

    class StateFuture : public StateBase {
    public:
        enum class InitType : uint8_t {
            None,
            Initial,
            Final,
        };
        enum class ResultType : uint8_t {
            None,
            Value,
            Exception,
        };
        typedef spinlock LockType;
        void DestroyDeallocate() override;
        void Resume() override;
        bool HasHandler() const override;
        StateBase *GetParent() const override;
        inline bool IsReady() const {
            if (_offset_of(StateFuture, is_future_) - _offset_of(StateFuture, hase_value_) == 1) {
                return 0 != reinterpret_cast<const std::atomic<uint16_t> &>(hase_value_).load(std::memory_order_acquire);
            } else {
                return hase_value_.load(std::memory_order_acquire) != ResultType::None || is_future_;
            }
        }
        inline bool HasHandlerSkipLock() const {
            return coro_ || is_init_co_ != InitType::None;
        }

        inline uint32_t GetAllocSize() const {
            return alloc_size_;
        }

        inline bool FutureAwaitReady() const {
            return hase_value_.load(std::memory_order_acquire) != ResultType::None;
        }

        template<class PromiseT, typename = std::enable_if<traits::IsPromiseV<PromiseT>>>
        void FutureAwaitSuspend(coroutine_handle<PromiseT> handler)

        bool SwitchSchedulerAwaitSuspend(Scheduler* sch);

        template<class PromiseT, typename = std::enable_if<traits::IsPromiseV<PromiseT>>>
        void PromiseInitialSuspend(coroutine_handle<PromiseT> handler);

        template<class PromiseT, typename = std::enable_if<traits::IsPromiseV<PromiseT>>>
        void PromiseFinalSuspend(coroutine_handle<PromiseT> handler);

        template<class Sty>
        static Sty* Construct(void* ptr, size_t size) {
            Sty* st = new(ptr) Sty(false);
            st->alloc_size_ = static_cast<uint32_t>(size);
            return st;
        }
        template<class Sty>
        static inline Sty* AllocState(bool awaiter) {
            AllocChar al;
            size_t size = AlignSize<Sty>();
#ifdef LIBCORO_DEBUG
            printf("StateFuture::AllocState, size=%d\n", size);
#endif
            char* ptr = al.allocate(size);
            Sty* st = new(ptr) Sty(awaiter);
            st->alloc_size = static_cast<uint32_t>(size);
            return st;
        }


    protected:
        explicit StateFuture(bool await) {
#ifdef LIBCORO_DEBUG
            id_ = ++g_coro_state_id;
#endif
            is_future_ = !await;
        }

        mutable LockType mtx_;
        coroutine_handle<> init_co_;
        StateFuture* parent_ = nullptr;
#ifdef LIBCORO
        intptr_t id_;
#endif
        uint32_t alloc_size_ = 0;
        std::atomic<ResultType> hase_value_ { ResultType::None };
        bool is_future_;
        InitType is_init_co_ = InitType::None;
        static_assert(sizeof(std::atomic<ResultType>) == 1);
        static_assert(alignof(std::atomic<ResultType>) == 1);
        static_assert(sizeof(bool) == 1);
        static_assert(alignof(bool) == 1);
        static_assert(sizeof(std::atomic<InitType>) == 1);
        static_assert(alignof(std::atomic<InitType>) == 1);
    private:
    };

    template <typename Tp>
    struct State final : public StateFuture {
    public:
        using StateFuture::LockType;
        using ValueType = Tp;

        ~State() override {
            switch (hase_value_.load(std::memory_order_acquire)) {
                case ResultType::Value:
                    value_.~ValueType();
                    break;
                case ResultType::Exception:
                    exception_.~exception_ptr();
                    break;
                default:
                    break;
            }
        }

        ValueType FutureAwaitResume();

        template<class PromiseT, class U, typename = std::enable_if<traits::IsPromiseV<PromiseT>>>
        void PromiseYieldValue(PromiseT* promise, U&& val);

        void SetException(std::exception_ptr e);

        template<typename U>
        void SetValue(U&& val);


        template<typename Exp>
        inline void ThrowException(Exp e) {
            SetException(std::make_exception_ptr(std::move(e)));
        }

        friend StateFuture;
    private:
        explicit State(bool await) : StateFuture(await) {}

        template<typename U>
        void SetValueInternal(U&& val);
        void SetExceptionInternal(std::exception_ptr e);
        union {
            std::exception_ptr exception_;
            ValueType value_;
        };

    };

    template <>
    class State<void> final : public StateFuture {
    public:
        friend StateFuture;
        using StateFuture::LockType;

        void FutureAwaitResume();

        template<class PromiseT, typename = std::enable_if<traits::IsPromiseV<PromiseT>>>
        void PromiseYieldValue(PromiseT* promise);
        void SetException(std::exception_ptr e);

        void SetValue();

        template<typename Exp>
        inline void ThrowException(Exp e) {
            SetException(std::make_exception_ptr(std::move(e)));
        }

        void DestroyDeallocate() override;

        void Resume() override;

        bool HasHandler() const override;

        StateBase *GetParent() const override;


    private:
        std::exception_ptr exception_;
    };










    // ------------------------------------
    template<class PromiseT, typename Enable>
    void StateFuture::PromiseInitialSuspend(coroutine_handle<PromiseT> handler) {
        PromiseT promise = handler.promise();
    }

















}




#endif //LIBCOROUTINE_STATE_H
