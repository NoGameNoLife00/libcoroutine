#ifndef LIBCOROUTINE_GENERATOR_H
#define LIBCOROUTINE_GENERATOR_H
#include <libcoro.h>
namespace libcoro {
    template <typename Tp, typename PromiseType>
    class GeneratorIterator;

    template <typename PromiseType>
    class GeneratorIterator<void, PromiseType> {
    public:
        typedef std::input_iterator_tag IteratorCategory;
        typedef ptrdiff_t DifferenceType;
        GeneratorIterator(std::nullptr_t) : coro_(nullptr) {}
        GeneratorIterator(coroutine_handle<PromiseType> coro) : coro_(coro) {}

        GeneratorIterator& operator++() {
            if (coro_.done()) {
                coro_ = nullptr;
            } else {
                coro_.resume();
            }
            return *this;
        }

        void operator++(int) {
            ++* this;
        }

        bool operator==(GeneratorIterator const& r) const {
            return coro_ == r.coro_;
        }
        bool operator!=(GeneratorIterator const& r) const {
            return !(*this == r);
        }
        coroutine_handle<PromiseType> coro_;
    };

    template <typename PromiseType>
    class GeneratorIterator<std::nullptr_t, PromiseType> : public GeneratorIterator<void, PromiseType> {
    public:
        GeneratorIterator(std::nullptr_t) : GeneratorIterator<void, PromiseType>(nullptr) {}
        GeneratorIterator(coroutine_handle<PromiseType> coro) : GeneratorIterator<void, PromiseType>(coro) {}
    };

    template <typename Tp, typename PromiseType>
    class GeneratorIterator : public GeneratorIterator<void, PromiseType> {
    public:
        using ValueType = Tp;
        using Refrence = Tp const&;
        using Pointer = Tp const*;

        GeneratorIterator(std::nullptr_t) : GeneratorIterator<void, PromiseType>(nullptr) {}
        GeneratorIterator(coroutine_handle<PromiseType> coro) : GeneratorIterator<void, PromiseType>(coro) {}

        Refrence operator*() const {
            return *this->coro.promise().current_value;
        }
        Pointer operator->() const {
            return this->coro_.promise().current_value;
        }
    };

    template <typename Tp, typename Alloc>
    class Generator {
    public:
        using ValueType = Tp;
        using StateType = StateGenerator;

        struct promise_type {
            using ValueType = Tp;
            using StateType = StateGenerator;
            using FutureType = Generator<ValueType>;

            Tp const* current_value;
            promise_type() {
                GetState()->SetInitialSuspend(coroutine_handle<promise_type>::from_promise(*this));
            }

            promise_type(promise_type&&) = default;
            promise_type(const promise_type&) = default;
            promise_type& operator=(promise_type&&) = default;
            promise_type& operator=(const promise_type&) = default;

            Generator get_return_object() {
                return Generator{*this};
            }

            suspend_always initial_suspend() noexcept {
                return {};
            }

            suspend_always yield_suspend(Tp const& val) noexcept {
                current_value = std::addressof(val);
                return {};
            }

            suspend_always final_suspend() noexcept {
                return {};
            }

            void return_value(Tp const& val) noexcept {
                current_value = std::addressof(val);
            }

            void return_value() noexcept {
                current_value = nullptr;
            }

            void set_exception(std::exception_ptr e) {
                std::terminate();
            }

            void unhandled_exception() {
                std::terminate();
            }

            template <typename Uty>
            Uty&& await_transform(Uty&& what) noexcept  {
                static_assert(std::is_same_v<Uty, void>,
                        "co_await is not supported in coroutines of type std::experiemental::generator_t");
                return std::forward<Uty>(what);
            }

            StateType* GetState() noexcept {
                size_t state_size = AlignSize<StateType>();
                auto h = coroutine_handle<promise_type>::from_promise(*this);
                char* ptr = reinterpret_cast<char*>(h.address()) - state_size;
                return reinterpret_cast<StateType*>(ptr);
            }

            StateType* RefState() {
                return GetState();
            }

            using AllocChar = typename std::allocator_traits<Alloc>::template rebind_alloc<char>;
            static_assert(std::is_same_v<char*, typename std::allocator_traits<AllocChar>::pointer>,
                    "generator_t does not support allocators with fancy pointer types");
            static_assert(std::allocator_traits<AllocChar>::is_always_equal::value,
                    "generator_t only supports stateless allocators");
            void* operator new(size_t size) {
                AllocChar al;
                size_t state_size = AlignSize<StateType>();
                assert(size > sizeof(uint32_t) && size < (std::numeric_limits<uint32_t>::max)() - sizeof(state_size));

                char* ptr = al.allocate(size + state_size);
                char* r_ptr = ptr + state_size;
#if LIBCORO_DEBUG
                printf("generator_promise::new, alloc size=%d\n", size + state_size);
                printf("generator_promise::new, alloc ptr=%x\n", (void*)ptr);
                printf("generator_promise::new, alloc return_ptr=%x\n", (void*)r_ptr);
#endif
                {
                    StateType* st = StateType::Construct(ptr);
                    st->lock();
                }
                return r_ptr;
            }

            void operator delete(void* ptr, size_t size) {
                size_t state_size = AlignSize<StateType>();
                assert(size >= sizeof(uint32_t) && size < (std::numeric_limits<uint32_t>::max)() - sizeof(state_size));
                *reinterpret_cast<uint32_t*>(ptr) = static_cast<uint32_t>(size+state_size);
                StateType* st = reinterpret_cast<StateType*>(static_cast<char*>(ptr) - state_size);
                st->unlock();
            }
        };

        using  iterator = GeneratorIterator<Tp, promise_type>;
        iterator begin() {
            if (coro_) {
                coro_.resume();
                if (coro_.done()) {
                    return {nullptr};
                }
            }
            return {coro_};
        }

        iterator end() {
            return {nullptr};
        }

        explicit Generator(promise_type& promise) :
        coro_(coroutine_handle<promise_type>::from_promise(promise)) {
        }

        Generator() = default;
        Generator(Generator const&) = delete;
        Generator& operator=(Generator const&) = delete;

        Generator(Generator&& r) : coro_(r.coro_) {
            r.coro_ = nullptr;
        }

        Generator& operator=(Generator&& r) {
            if (this != std::addressof(r)) {
                coro_ = r.coro_;
                r.coro_ = nullptr;
            }
            return *this;
        }

        ~Generator() {
            if (coro_) {
                coro_.destroy();
            }
        }

        StateType* DetachState() {
            auto t = coro_;
            coro_ = nullptr;
            return t.promise().GetState();
        }
    private:
        coroutine_handle<promise_type> coro_ = nullptr;
    };
}

#endif //LIBCOROUTINE_GENERATOR_H
