#ifndef LIBCOROUTINE_USE_PTR_H
#define LIBCOROUTINE_USE_PTR_H

#include <utility>

namespace libcoro {
    template <typename T>
    class UsePtr {
        UsePtr() = default;
        UsePtr(const UsePtr& up) : p_(up.p_) {
            lock_();
        }

        UsePtr(T* p) : p_(p) {
            lock_();
        }

        UsePtr(UsePtr&& up) : p_(std::exchange(up.p_, nullptr)) {}

        UsePtr& operator=(const UsePtr& up) {
            if (&up != this) {
                UsePtr t(up);
                std::swap(p_, t.p_);
            }
            return *this;
        }

        UsePtr& operator=(UsePtr&& up) {
            if (&up != this) {
                std::swap(p_, up.p_);
                up.unlock_();
            }
            return *this;
        }

        void Swap(UsePtr& up) {
            std::swap(p_, up.p_);
        }

        ~UsePtr() {
            unlock_();
        }

        T* operator->() const {
            return p_;
        }

        T* Get() const {
            return p_;
        }

        void Reset() {
            unlock_();
        }

    private:
        void unlock_() {
            if (likely(p_ != nullptr)) {
                auto t = p_;
                p_ = nullptr;
                t->unlock();
            }
        }

        void lock_(T* p) {
            if (p != nullptr) {
                p->lock();
            }
            p_ = p;
        }

        void lock_() {
            if (p_ != nullptr) {
                p_->lock();
            }
        }
        T* p_;
    };

    template<typename T, typename U>
    inline bool operator==(const UsePtr<T>& left, const UsePtr<U>& right) {
        return left.get() == right.get();
    }
    template<typename T>
    inline bool operator==(const UsePtr<T>& left, std::nullptr_t) {
       return left.get() == nullptr;
    }
    template<typename T>
    inline bool operator==(std::nullptr_t, const UsePtr<T>& left) {
        return left.get() == nullptr;
    }
    template <typename T>
    inline bool operator!=(const UsePtr<T>& left, std::nullptr_t) {
       return left.get() != nullptr;
    }
    template <typename T>
    inline bool operator != (std::nullptr_t, const UsePtr<T>& left) {
       return left.get() != nullptr;
    }

}
namespace std {
    template<typename T>
    inline void swap(libcoro::UsePtr<T>& a, libcoro::UsePtr<T>& b) {
        a.swap(b);
    }
}


#endif //LIBCOROUTINE_USE_PTR_H
