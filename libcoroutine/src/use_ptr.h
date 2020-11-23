#ifndef LIBCOROUTINE_USE_PTR_H
#define LIBCOROUTINE_USE_PTR_H

#include <libcoro.h>

namespace libcoro {
    template <typename T>
    class use_ptr {
    public:
        use_ptr() = default;
        use_ptr(const use_ptr& up) : p_(up.p_) {
            lock_();
        }

        use_ptr(T* p) : p_(p) {
            lock_();
        }

        use_ptr(use_ptr&& up) : p_(std::exchange(up.p_, nullptr)) {}

        use_ptr& operator=(const use_ptr& up) {
            if (&up != this) {
                use_ptr t(up);
                std::swap(p_, t.p_);
            }
            return *this;
        }

        use_ptr& operator=(use_ptr&& up) {
            if (&up != this) {
                std::swap(p_, up.p_);
                up.unlock_();
            }
            return *this;
        }

        void swap(use_ptr& up) {
            std::swap(p_, up.p_);
        }

        T* operator->() const {
            return p_;
        }

        T* get() const {
            return p_;
        }

        void reset() {
            unlock_();
        }

        ~use_ptr() {
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
    inline bool operator==(const use_ptr<T>& left, const use_ptr<U>& right) {
        return left.get() == right.get();
    }
    template<typename T>
    inline bool operator==(const use_ptr<T>& left, std::nullptr_t) {
       return left.get() == nullptr;
    }
    template<typename T>
    inline bool operator==(std::nullptr_t, const use_ptr<T>& left) {
        return left.get() == nullptr;
    }
    template <typename T>
    inline bool operator!=(const use_ptr<T>& left, std::nullptr_t) {
       return left.get() != nullptr;
    }
    template <typename T>
    inline bool operator != (std::nullptr_t, const use_ptr<T>& left) {
       return left.get() != nullptr;
    }

}
namespace std {
    template<typename T>
    inline void swap(libcoro::use_ptr<T>& a, libcoro::use_ptr<T>& b) {
        a.swap(b);
    }
}


#endif //LIBCOROUTINE_USE_PTR_H
