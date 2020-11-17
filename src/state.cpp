
#include "state.h"

namespace libcoro {
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
                scheduler_;
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
        return false;
    }
}