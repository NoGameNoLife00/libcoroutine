
#include "state.h"
#include <scheduler.h>

namespace libcoro {

    void StateBase::DestroyDeallocate() {
        delete this;
    }

        void StateGenerator::DestroyDeallocate() {
        size_t size = AlignSize<StateGenerator>();
        char* ptr = reinterpret_cast<char*>(this) + size;
        size = *reinterpret_cast<uint32_t*>(ptr);

#if LIBCORO_DEBUG
        printf("DestroyDeallocate, size=%d\n", size);
#endif
        this->~StateGenerator();
        AllocChar al;
        return al.deallocate(reinterpret_cast<char*>(this), size);
    }

    void StateGenerator::Resume() {
        if (likely(coro_)) {
            coro_.resume();
            if (likely(!coro_.done())) {
                scheduler_->AddGenerator(this);
            } else {
                coroutine_handle<> handler = coro_;
                coro_ = nullptr;
                scheduler_->DelFinal(this);
                handler.destroy();
            }
        }
    }

    StateGenerator *StateGenerator::AllocState() {
        AllocChar al;
        size_t size = AlignSize<StateGenerator>();
#if LIBCORO_DEBUG
        printf("StateGenerator::AllocState, size=%d\n", sizeof(StateGenerator));
#endif
        char* ptr = al.allocate(size);
        return new(ptr) StateGenerator();
    }

    bool StateGenerator::HasHandler() const {
        return (bool)coro_;
    }

    bool StateGenerator::SwitchSchedulerAwaitSuspend(Scheduler* sch) {
        assert(sch);
        if (scheduler_) {
            if (scheduler_ == sch) {
                return false;
            }
            auto task = scheduler_->DelSwitch(this);
            scheduler_ = sch;
            if (task) {
                sch->AddSwitch(std::move(task));
            }
        } else {
            scheduler_ = sch;
        }
        return true;
    }

    void StateFuture::Resume() {
        std::unique_lock<LockType> guard(mtx_);
        if (is_init_co_ == InitType::Initial) {
            assert((bool)init_co_);
            coroutine_handle<> handler = init_co_;
            is_init_co_ = InitType::None;
            guard.unlock();
            handler.resume();
            return;
        }
        if (coro_) {
            coroutine_handle<> handle = coro_;
            coro_ = nullptr;
            guard.unlock();

            handle.resume();
            return;
        }
        if (is_init_co_ == InitType::Final) {
            assert((bool)init_co_);
            coroutine_handle<> handler = init_co_;
            is_init_co_ = InitType::None;
            guard.unlock();

            handler.resume();
            return;
        }
    }

    bool StateFuture::HasHandler() const {
        scoped_lock<LockType> guard(mtx_);
        return HasHandlerSkipLock();
    }

    StateBase *StateFuture::GetParent() const {
        return StateBase::GetParent();
    }

    void StateFuture::DestroyDeallocate() {
        size_t size = alloc_size_;
#ifdef LIBCORO_DEBUG
        printf("DestroyDeallocate, size=%d\n", size);
#endif
        this->~StateFuture();
        AllocChar al;
        return al.deallocate(reinterpret_cast<char*>(this), size);
    }

    bool StateFuture::SwitchSchedulerAwaitSuspend(Scheduler *sch) {
        assert(sch);
        scoped_lock<LockType> guard(mtx_);
        if (scheduler_) {
            if (scheduler_ == sch) {
                return false;
            }
            auto task = scheduler_->DelSwitch(this);
            scheduler_ = sch;
            if (task) {
                sch->AddSwitch(std::move(task));
            }
        } else {
            scheduler_ = sch;
        }

        if (parent_) {
            parent_->SwitchSchedulerAwaitSuspend(sch);
        }
        return true;
    }

    void State<void>::SetValue() {
        scoped_lock<LockType> guard(mtx_);
        has_value_.store(ResultType::Value, std::memory_order_release);
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