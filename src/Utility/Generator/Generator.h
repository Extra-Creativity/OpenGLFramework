#pragma once
#ifndef OPENGLFRAMEWORK_GENERATOR_H_
#define OPENGLFRAMEWORK_GENERATOR_H_

#include <coroutine>
#include <optional>
#include <iterator>

namespace OpenGLFramework::Coroutine {

template<std::movable T>
class Generator
{
public:
    struct promise_type
    {
        Generator<T> get_return_object()
        {
            return Generator{Handle::from_promise(*this)};
        }
        static std::suspend_always initial_suspend() noexcept { return {};}
        static std::suspend_always final_suspend() noexcept { return {};}
        std::suspend_always yield_value(T value) noexcept
        {
            currentVal = std::move(value);
            return {};
        }
        // Disallow co_await in generator coroutines.
        void await_transform() = delete;
        [[noreturn]] static void unhandled_exception() { throw; }
        void return_void() noexcept {}

        std::optional<T> currentVal;
    };
 
    using Handle = std::coroutine_handle<promise_type>;
 
    explicit Generator(const Handle coroutine) : coroutine_{coroutine} {}
 
    Generator() = default;
    ~Generator()
    {
        if (coroutine_)
            coroutine_.destroy();
    }
 
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
 
    Generator(Generator&& other) noexcept : coroutine_{other.coroutine_}
    {
        other.coroutine_ = {};
    }
    Generator& operator=(Generator&& other) noexcept
    {
        if (this != &other)
        {
            if (coroutine_)
                coroutine_.destroy();
            coroutine_ = other.coroutine_;
            other.coroutine_ = {};
        }
        return *this;
    }
 
    // Range-based for loop support.
    class Iter
    {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef T                         value_type;
        typedef std::ptrdiff_t            difference_type;
        typedef T*                        pointer;
        typedef T&                        reference;

        void operator++()
        {
            coroutine_.resume();
        }
        void operator++(int) { operator++(); }
        const T& operator*() const
        {
            return *coroutine_.promise().currentVal;
        }

        const T* operator->() const
        {
            return &(coroutine_.promise().currentval);
        }

        bool operator==(std::default_sentinel_t) const
        {
            return !coroutine_ || coroutine_.done();
        }
 
        explicit Iter(const Handle coroutine) : coroutine_{coroutine} {}
 
    private:
        Handle coroutine_;
    };
 
    Iter begin()
    {
        if (coroutine_)
            coroutine_.resume();
        return Iter{coroutine_};
    }
 
    std::default_sentinel_t end() { return {}; }
 
private:
    Handle coroutine_;
};

} // namespace OpenGLFramework::Coroutine

#endif // !OPENGLFRAMEWORK_GENERATOR_H_