// variable template example

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <tlx/meta.hpp>

//! print function outputting any type
template <typename Type>
void print(const Type& t) {
    std::cout << t << std::endl;
}

//! print function with any number of parameters of any type.
template <typename Type, typename... MoreTypes>
void print(const Type& t, const MoreTypes&... more) {
    // print first via the base case
    print(t);
    // recursively process the remainder
    print(more...);
}

//! another function taking any number of parameters
template <typename... Types>
void test1(Types&... values) {
    // find the number of parameters
    size_t x = sizeof...(Types);
    std::cout << "sizeof=" << x << std::endl;

    // print values.
    print(values.front()...);

    // expand parameter list, but call .size() them.
    print(values.size()...);

    // one of the kludges: execute functions on each parameter
    using VarForeachExpander = int[];
    (void)VarForeachExpander{(values.shrink_to_fit(), 0)...};

    // alternative: execute void() function, but gobble unused 0
    tlx::vexpand((values.shrink_to_fit(), 0)...);

    // create std::tuple matching types
    std::tuple<typename Types::value_type...> t;
    t = std::make_tuple(values.front()...);

    // size of tuple
    std::cout << "tuple_size=" << std::tuple_size<decltype(t)>() << std::endl;

    // append another component
    auto t2 = std::tuple_cat(t, std::make_tuple(42));

    // this is hard: apply a variadic function to a tuple. needs more magic in
    // tlx::apply_tuple, also needs print to be parameterized
    tlx::apply_tuple(print<typename Types::value_type..., int>, t2);
}

//! what can be done with the print() function, can also be done with template
//! classes. this is the template prototype declaration:
template <typename... Types>
class VTClass;

//! template specialization which takes at least one Type.
template <typename Type, typename... Types>
class VTClass<Type, Types...> {
public:
    VTClass(const Type& value, const Types&... rest)
        : value_(value), rest_(rest...) {}

    Type value_;

    //! somehow store the rest
    VTClass<Types...> rest_;
};

//! template specialization with zero Types as base case.
template <>
class VTClass<> {};

//! another class parameterized with any number of Types.
template <typename... Types>
class VTSimple {
public:
    VTSimple(const Types&... values) {
        // process parameters using a fold expression (+) - this requires gcc 6:
        // print((values + ...));

        // more from tlx's meta library: apply a generic lambda to each of
        // value's components
        tlx::call_foreach(
            [](const auto& v) { std::cout << v << std::endl; }, values...);

        tlx::call_foreach_with_index(
            [](const auto& index, const auto& v) {
                std::cout << index << ": " << v << std::endl;
            },
            values...);

        tuple_ = std::make_tuple(values...);
    }

    void run() {
        // again tlx's meta library: expand tuple and apply a generic lambda to
        // each component.
        tlx::call_foreach_tuple_with_index(
            [](const auto& index, const auto& v) {
                std::cout << index << ": " << v << std::endl;
            },
            tuple_);
    }

    std::tuple<Types...> tuple_;
};

//! most common use case of variadic universal references: constructors. the
//! advance is that one does not need to explicitly state the argument types
//! when calling.
template <typename... Types>
VTSimple<Types...> make_vtsimple(Types&&... arguments) {
    // make sure to keep l-value/r-value when forwarding (perfect forward)
    return VTSimple<Types...>(std::forward<Types>(arguments)...);
}

int main() {
    // call print with some arguments
    print(5);
    print(5, "hello", 42.0);

    // call test1() with some types, they must support .size() and other
    // functions called within test1()
    std::vector<int> vec{1, 2, 3, 4};
    std::string str = "hello";
    test1(vec, str);

    // construct a VTClass
    VTClass<int, float> vt1(5, 5.0);

    // construct object
    VTSimple<int, float> abc(5, 5.0);

    // using constructor function: infer arguments universal references
    auto def = make_vtsimple(5, 5.0);
    def.run();

    return 0;
}
