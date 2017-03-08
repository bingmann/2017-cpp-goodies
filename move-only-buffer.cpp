// move-only Buffer example

#include <functional>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>

#include <tlx/delegate.hpp>

//! A non-copyable move-only buffer which contains a "large" memory area
class Buffer {
public:
    //! allocate buffer containing n bytes
    explicit Buffer(size_t n)
        : data_(reinterpret_cast<char*>(operator new(n))), size_(n) {}

    //! allocate buffer containing string str
    explicit Buffer(const char* str) : Buffer(strlen(str)) {
        std::copy(str, str + size_, data_);
    }

    //! non-copyable: delete copy-constructor
    Buffer(const Buffer&) = delete;
    //! non-copyable: delete assignment operator
    Buffer& operator=(const Buffer&) = delete;

    //! move-construct other buffer into this one
    Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    //! move-assignment of other buffer into this one
    Buffer& operator=(Buffer&& other) noexcept {
        if (this == &other)
            return *this;

        operator delete(data_);
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;

        return *this;
    }

    //! delete buffer
    ~Buffer() { operator delete(data_); }

    //! return as string
    std::string to_string() const { return std::string(data_, size_); }

    //! l-value this
    void test() &
    { std::cout << "test: 'this' is a l-value" << std::endl; }

    //! r-value this
    void test() &&
    { std::cout << "test: 'this' is a r-value" << std::endl; }

private:
    //! the buffer
    char* data_;
    //! buffer size
    size_t size_;
};

//! a "real" send function called by the "facade" send functions below. this is
//! the const lvalue reference version, which internally has to copy the buffer
void real_send(const Buffer& b) {
    std::cout << "real_send (l-value ref): " << b.to_string() << std::endl;
}

//! a "real" send function called by the "facade" send functions below. this is
//! the (mutable) rvalue reference version, which can acquire the buffer's
//! content without copying using a move operation.
void real_send(Buffer&& b) {
    std::cout << "real_send (r-value ref): " << b.to_string() << std::endl;
}

//! function called by value
void send1(Buffer b) {
    /* send ... */
    std::cout << b.to_string() << std::endl;
}

//! function called by mutable l-value reference
void send2(Buffer& b) {
    /* send ... */
    std::cout << b.to_string() << std::endl;
}

//! function called by const l-value reference
void send3(const Buffer& b) {
    /* send ... */
    std::cout << b.to_string() << std::endl;
}

//! function called by r-value reference
void send4(Buffer&& b) {
    /* send ... */
    std::cout << b.to_string() << std::endl;
}

//! function making a Buffer -- the return value is automatically an rvalue.
Buffer make_buffer() {
    Buffer b("new buffer");
    // !!! never write: return std::move(b);
    return b;
}

//! "universal" reference
template <typename Type>
void work_buffer(Type&& value)
{
    // std::forward is used to cast value to lvalue/rvalue depending on how this
    // function is called. this enables perfect forwarding of l/r-value-ness.
    std::forward<Type>(value).test();
}

//! "long" version of a lambda: a functor
class Functor
{
public:
    Functor(Buffer&& b1) : b1(std::move(b1)) { }

    void operator () () const {
        std::cout << b1.to_string() << std::endl;
    }

private:
    Buffer b1;
};

int main() {
    // which send1/2/3/4 can be called?
    {
        Buffer b1("buffer1"), b2("buffer2"), b3("buffer3"), b4("buffer4");

        // send1(b1);              // error: not copyable
        send2(b1);                 // ok: mutable ref to an lvalue
        send3(b1);                 // ok: const ref to an lvalue
        // send4(b1);              // error: cannot bind lvalue to rvalue

        send1(std::move(b1));      // ok: use move constructor to call by value
        // send2(std::move(b2));   // error: non-const reference to rvalue
        send3(std::move(b3));      // ok: const reference to rvalue
        send4(std::move(b4));      // ok: pass rvalue by reference

        send1(Buffer("temporary r-value"));     // ok: use move constructor
        // send2(Buffer("temporary r-value"));  // error: non-const reference
        send3(Buffer("temporary r-value"));     // ok: const reference to rvalue
        send4(Buffer("temporary r-value"));     // ok: pass rvalue by reference
    }

    // distinguish if this is l-value or r-value
    {
        Buffer lvalue("l-value this");
        lvalue.test();

        Buffer("r-value this").test();
    }

    // move Buffer into closure of lambda
    {
        Buffer bl("lambda buffer");

        auto print_lambda = [bl = std::move(bl)]() {
            // send1(bl);
            // send2(bl);
            send3(bl);
            // send4(bl);
        };

        print_lambda();
    }

    // use Buffer in closure of mutable lambda
    {
        Buffer bl("mutable lambda buffer");

        auto print_lambda = [bl = std::move(bl)]() mutable {
            send1(std::move(bl));
            send2(bl);
            send3(bl);
            send4(std::move(bl));
        };

        print_lambda();
    }

    // try with std::function<void()>
    {
        Buffer bl("std::function buffer");

        // cannot move bl into lambda's closure
        std::function<void()> print_std_function = [&bl]() {
            std::cout << bl.to_string() << std::endl;
        };

        print_std_function();
    }

    // try with tlx::delegate<void()>: works because it supports move-only
    // closures
    {
        Buffer bl("std::function buffer");

        tlx::delegate<void()> print_std_function = [bl = std::move(bl)]() {
            std::cout << bl.to_string() << std::endl;
        };

        print_std_function();
    }

    return 0;
}
