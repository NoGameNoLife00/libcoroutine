
#include <libcoro.h>
#include "state.h"


namespace libcoro {
    template<PromiseT PromiseTp>
    void StateFuture::PromiseInitialSuspend(coroutine_handle<PromiseTp> handler) {
        assert(scheduler_ == nullptr);
        assert(!coro_);

        init_co_ = handler;
        is_init_co_ = InitType::Initial;
    }

    template<PromiseT PromiseTp>
    void StateFuture::PromiseFinalSuspend(coroutine_handle<PromiseTp> handler) {
        scoped_lock<LockType> guard(mtx_);
        init_co_ = handler;
        is_init_co_ = InitType::Final;

        Scheduler* sch = GetScheduler();
        assert(sch);

        if (HasHandlerSkipLock()) {
            sch->AddGenerator(this);
        }
        sch->DelFinal(this);
    }


    template<PromiseT PromiseTp>
    void StateFuture::FutureAwaitSuspend(coroutine_handle<PromiseTp> handler) {
        PromiseTp& promise = handler.promise();
        auto* parent_state = promise.GetState();
        Scheduler* sch = parent_state->GetScheduler();
        scoped_lock<LockType> guard(mtx_);
        if (this != parent_state) {
            parent_ = parent_state;
            scheduler_ = sch;
        }
        if (!coro_) {
            coro_ = handler;
        }
        if (sch != nullptr && IsReady()) {
            sch->AddGenerator(this);
        }
    }

    template<typename Tp>
    template<PromiseT PromiseTp, class U>
    void State<Tp>::PromiseYieldValue(PromiseTp *promise, U &&val) {
        coroutine_handle<PromiseTp> handler = coroutine_handle<PromiseTp>::from_promise(*promise);

        scoped_lock<LockType> guard(mtx_);
        if (!handler.done()) {
            if (!coro_) {
                coro_ = handler;
            }
        }
        SetValueInternal(std::forward<U>(val));
        if (!handler.done()) {
            Scheduler* sch = GetScheduler();
            if (sch) {
                sch->AddGenerator(this);
            }
        }
    }

    template<typename Tp>
    template<typename U>
    void State<Tp>::SetValue(U &&val) {
        scoped_lock<LockType> guard(mtx_);
        SetValueInternal(std::forward<U>(val));

        Scheduler* sch = GetScheduler();
        if (sch) {
            if (HasHandlerSkipLock()) {
                sch->AddGenerator(this);
            } else {
                sch->DelFinal(this);
            }
        }

    }

    template<typename Tp>
    template<typename U>
    void State<Tp>::SetValueInternal(U &&val) {
        switch (has_value_.load(std::memory_order_acquire)) {
            case ResultType::Value:
                value_ = std::forward<U>(val);
                break;
            case ResultType::Exception:
                exception_.~exception_ptr();
                break;
            default:
                new (&value_) ValueType(std::forward<U>(val));
                has_value_.store(ResultType::Value, std::memory_order_release);
                break;
        }
    }

    template<typename Tp>
    void State<Tp>::SetExceptionInternal(std::exception_ptr e) {
        switch (has_value_.load(std::memory_order_acquire)) {
            case ResultType::Exception:
                exception_ = std::move(e);
                break;
            case ResultType::Value:
                value_.~ValueType();
            default:
                new (&exception_) std::exception_ptr(std::move(e));
                has_value_.store(ResultType::Exception, std::memory_order_release);
                break;
        }
    }

    template<PromiseT PromiseTp>
    void State<void>::PromiseYieldValue(PromiseTp* promise) {
        coroutine_handle<PromiseTp> handler = coroutine_handle<PromiseTp>::from_promise(*promise);
        scoped_lock<LockType> guard(mtx_);
        if (!handler.done()) {
            if (!coro_) {
                coro_ = handler;
            }
        }
        has_value_.store(ResultType::Value, std::memory_order_release);
        if (!handler.done()) {
            Scheduler* sch = GetScheduler();
            if (sch) {
                sch->AddGenerator(this);
            }
        }
    }

    template <typename Tp>
    void State<Tp>::SetException(std::exception_ptr e) {
        scoped_lock<LockType> guard(mtx_);
        SetExceptionInternal(std::move(e));

        Scheduler* sch = GetScheduler();
        if (sch) {
            if (HasHandlerSkipLock()) {
                sch->AddGenerator(this);
            } else {
                sch->DelFinal(this);
            }
        }
    }

    template<typename Tp>
    auto State<Tp>::FutureAwaitResume() -> ValueType {
        scoped_lock<LockType> guard(mtx_);

        switch (has_value_.load(std::memory_order_acquire)) {
            case ResultType::None:
                std::rethrow_exception(std::make_exception_ptr(
                        FutureException{ErrorCode::NotReady }));
            case ResultType::Exception:
                std::rethrow_exception(std::exchange(exception_, nullptr));
                break;
            default:
                break;
        }
        return std::move(value_);
    }

    template<typename Tp>
    template<PromiseT PromiseTp>
    void State<Tp &>::PromiseYieldValue(PromiseTp *promise, RefrenceType val) {
        coroutine_handle<PromiseTp> handler = coroutine_handle<PromiseTp>::from_promise(*promise);
        scoped_lock<LockType> guard(mtx_);
        if (!handler.done()) {
            if (!coro_) {
                coro_ = handler;
            }
        }
        SetValueInternal(val);
        if (!handler.done()) {
            Scheduler* sch = GetScheduler();
            sch->AddGenerator(this);
        }
    }

    template<typename Tp>
    void State<Tp &>::SetValue(RefrenceType val) {
        scoped_lock<LockType> guard(mtx_);
        SetValueInternal(val);

        Scheduler* sch = GetScheduler();
        if (sch) {
            if (HasHandlerSkipLock()) {
                sch->AddGenerator(this);
            } else {
                sch->DelFinal(this);
            }
        }
    }

    template<typename Tp>
    void State<Tp &>::SetValueInternal(RefrenceType val) {
        switch (has_value_.load(std::memory_order_acquire)) {
            case ResultType::Exception:
                exception_.~exception_ptr();
            default:
                value_ = std::addressof(val);
                has_value_.store(ResultType::Value, std::memory_order_release);
                break;
        }
    }

    template <typename Tp>
    auto State<Tp&>::FutureAwaitResume() -> RefrenceType {
        scoped_lock<LockType> guard(mtx_);

        switch (has_value_.load(std::memory_order_acquire)) {
            case ResultType::None:
                std::rethrow_exception(std::make_exception_ptr(FutureException{ErrorCode::NotReady}));
                break;
            case ResultType::Exception:
                std::rethrow_exception(std::move(exception_));
                break;
            default:
                break;
        }
        return static_cast<RefrenceType>(*value_);
    }

    template<typename Tp>
    void State<Tp&>::SetExceptionInternal(std::exception_ptr e) {
        switch (has_value_.load(std::memory_order_acquire)) {
            case ResultType::Exception:
                exception_ = std::move(e);
                break;
            default:
                new (&exception_) std::exception_ptr(std::move(e));
                has_value_.store(ResultType::Exception, std::memory_order_release);
                break;
        }
    }

    template <typename Tp>
    void State<Tp&>::SetException(std::exception_ptr e) {
        scoped_lock<LockType> guard(mtx_);
        SetExceptionInternal(std::move(e));
        Scheduler* sch = GetScheduler();
        if (sch) {
            if (HasHandlerSkipLock()) {
                sch->AddGenerator(this);
            } else {
                sch->DelFinal(this);
            }
        }
    }
}

