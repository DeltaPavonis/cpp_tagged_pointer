/* Implements a type-safe hardware type-tagged pointer in C++20. Tagged pointers allow for
runtime polymorphism while avoiding the traditional storage overhead from virtual function
table pointers, which are stored for all pointers to an abstract base class. */

#include <cstdint>          // For `uintptr_t`
#include <type_traits>      // For `std::integral_constant`, `std::disjunction_v`
#include <utility>          // For `std::forward`
#include "dispatch_call.h"

namespace detail {

/* `IndexOfType` is a helper class that enables us to find the index of the type `T` within
a list of types given by the parameter pack `Ts`. */
template <typename T, typename... Ts>
struct IndexOfType;

/* Base case: the type `T` is the first type in the parameter pack `Ts`. In this case the
desired index is 0. */
template <typename T, typename... Ts>
struct IndexOfType<T, T, Ts...>
    : public std::integral_constant<unsigned, 0> {};

/* Recursion: If the type `T` is not the first type in the parameter pack `Ts`, then we remove
the first type of `Ts`, query the index of `T` in the new parameter pack, and add 1 to the
resulting index.  */
template <typename T, typename T2, typename... Ts>
struct IndexOfType<T, T2, Ts...>
    : public std::integral_constant<unsigned, 1 + IndexOfType<T, Ts...>::value> {};

/* `IndexOfType_v<T, Ts...>` equals the index of the type `T` within the list of types `Ts...`.
Zero-indexed, and determined at compile-time. */
template <typename T, typename... Ts>
constexpr unsigned IndexOfType_v = IndexOfType<T, Ts...>::value;

};  /* Ending bracket for `namespace detail` */

/* `TaggedPointer<Ts...>` represents a type-tagged pointer to one of the set of types specified by
the parameter pack `Ts...`. */
template <typename... Ts>
class TaggedPointer {
    /* Expect there to be enough space for the tag */
    static_assert(sizeof(uintptr_t) >= 8, "We expect `uintptr_t` to have at least 64 bits");

    /* `TAG_SHIFT` tells us how much to left-shift the tag when tagging an address. In other
    words, the least significant bit of the tag starts at bit `TAG_SHIFT` of `tagged_address`. */
    constexpr static unsigned TAG_SHIFT = 59;  /* Gives us 31 possible tags with 64-bit pointers */
    /* `GET_PTR_MASK` is the bitmask with the lowest `TAG_SHIFT` bits set to 1, and all other
    bits set to 0. Taking the bitwise OR of `GET_PTR_MASK` with `tagged_address` thus zeroes
    out the tag bits while keeping all the other bits unchanged, meaning that the result will
    be equal to the address of the original pointer (hence the name `GET_PTR_MASK`). */
    constexpr static uintptr_t GET_PTR_MASK = (static_cast<uintptr_t>(1) << TAG_SHIFT) - 1;

    /* `tagged_address` is simply the address of the tagged pointer. Specifically, the
    `TAG_SHIFT` least significant bits represent the true address of the untagged pointer,
    while the remaining bits are the tag. */
    uintptr_t tagged_address;

public:

    /* Returns the number of types this `TaggedPointer` can point to; that is, the size of
    the given parameter pack `Ts...`. */
    constexpr auto num_types() const {return sizeof...(Ts);}

    /* Returns the tag of the type `T`, which depends on the ordered parameter pack `Ts...`
    specified in the declaration of this `TaggedPointer`.
    
    A return value of `0` means this `TaggedPointer` was constructed with a `nullptr` (so
    it does not have a determined pointer type). Any other return value is guaranteed to
    fall in the range [1, num_types()], and will equal the one-indexed position of `T`
    relative to the parameter pack `Ts...`. */
    template <typename T>
    constexpr unsigned get_tag_of_type() const {
        /* Return `0` if this `TaggedPointer` was constructed with a `nullptr` */
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return 0;
        }
        /* Otherwise, return the one-indexed position of `T` within `Ts...`. `IndexOfType_v`
        returns a zero-indexed position, so we add one to make it an one-indexed position
        because a return value of 0 is used to represent the case of the type being
        `std::nullptr_t`, as explained above. */
        return 1 + detail::IndexOfType_v<T, Ts...>;
    }

    /* Returns the current tag of this `TaggedPointer`. */
    auto tag() const {return static_cast<unsigned>(tagged_address >> TAG_SHIFT);}
    
    /* Returns the address of the pointer stored in this `TaggedPointer` as a `void*`. */
    const void *ptr() const {return reinterpret_cast<void*>(tagged_address & GET_PTR_MASK);}
    /* Returns the address of the pointer stored in this `TaggedPointer` as a `void*`. */
    void *ptr() {return reinterpret_cast<void*>(tagged_address & GET_PTR_MASK);}

    /* Returns the pointer stored in this `TaggedPointer` casted to a `const T*` if the current
    type pointed to by this `TaggedPointer` is the same as `T`, and returns `nullptr` otherwise.
    
    To avoid the type equality check between `T` and the true current type pointed to by this
    `TaggedPointer`, use `cast_unsafe()` instead. */
    template <typename T>
    const T *cast() const {
        return is_of_type<T>() ? reinterpret_cast<const T*>(ptr()) : nullptr;
    }

    /* Returns the pointer stored in this `TaggedPointer` casted to a `T*` if the current
    type pointed to by this `TaggedPointer` is the same as `T`, and returns `nullptr` otherwise.
    
    To avoid the type equality check between `T` and the true current type pointed to by this
    `TaggedPointer`, use `cast_unsafe()` instead. */
    template <typename T>
    T *cast() {
        return is_of_type<T>() ? reinterpret_cast<T*>(ptr()) : nullptr;
    }

    /* Returns the pointer stored in this `TaggedPointer` casted to a `const T*`. Does not perform
    any check that this `TaggedPointer` truly points to an object of type `T`. */ 
    template <typename T>
    const T *cast_unsafe() const {
        return reinterpret_cast<const T*>(ptr());
    }

    /* Returns the pointer stored in this `TaggedPointer` casted to a `T*`. Does not perform any
    check that this `TaggedPointer` truly points to an object of type `T`. */ 
    template <typename T>
    T *cast_unsafe() {
        return reinterpret_cast<T*>(ptr());
    }

    /* Returns `true` iff the type pointed to by this `TaggedPointer` is the same as `T`. */
    template <typename T>
    bool is_of_type() const {return tag() == get_tag_of_type<T>();}

    /* Calls the function `func`, passing to it the pointer stored in this `TaggedPointer`,
    casted to the correct type, and returns the resulting value (with value category/cv-qualifiers
    preserved). `func` must have a single return type across all possible types pointed to by this
    `TaggedPointer`; if not, a compiler error will be raised. */
    template <typename Func>
    /* We use `decltype(auto)` as the return type. This is a C++14 feature that enables perfect
    forwarding of the return type; that is, it ensures that reference types are returned as
    reference types, and are not converted to their underlying value types (which is what would
    happen if we just used `auto` for the return type).

    You can understand this as follows. `decltype(x)` queries the declared type of the expression
    `x`, as well as its value category (lvalue, rvalue, etc) and its cv-qualifiers (whether it is
    `const`/`volatile` or not). `auto`, in the context of a function's return type, meanwhile,
    simply indicates that the function's return type shall be deduced from the function's return
    statements by the compiler.

    Thus, `decltype(auto)` tells the compiler to deduce the return type of the function, and to take
    into account the value category and cv-qualifiers of the expression returned when determining
    the return type. This allows `call` to return a reference, a plain value, a `const` type, etc,
    depending on the specific type/value category/cv-q qualifiers of the expression it returns. */
    decltype(auto) call(Func &&func) {
        /* The work of casting the underlying pointer to its correct type and passing the casted
        pointer to `func` is all done by `dispatch_call`.
        
        Note that the `std::forward` is necessary for the `func` parameter below. If this was
        omitted, then trying to pass in a lambda directly to `call`, such as in
        `my_tagged_ptr.call([](){});`, would result in a compiler error. This is because the lambda
        `[](){}` would be an rvalue (temporary lambda expressions are prvalues, according to
        cppreference: http://tinyurl.com/2z48u4dt), and so `func` would be a named rvalue reference.
        In C++, named rvalue references are lvalues, which means if you tried to pass `func` to
        `dispatch_call<Func, Ts...>` directly, you would be trying to pass in a `Func` to a
        function that expects a `Func&&` (as we explicitly specified the type as `Func` when
        calling `dispatch_call`, which prevents the `Func&&` in `dispatch_call`'s parameter
        list from being an universal reference (which can accept anything), and instead forces it
        to actually be a `Func&&`; an rvalue reference to `Func`). Now, because binding a rvalue
        reference to a lvalue is disallowed in C++, the compiler error occurs. Using `std::forward`
        solves the problem by preserving the value category (lvalue or rvalue) of the argument
        `func` when it is passed to `dispatch_call`; `func` is casted to an value if and only if
        it was passed in as a rvalue, and if `func` was passed in as a lvalue, it remains a lvalue.
        This makes a type mismatch between the `func` parameter of `dispatch_call<Func, Ts...>`
        and the `func` parameter of `call` impossible, resolving the issue. */
        return detail::dispatch_call<Func, Ts...>(std::forward<Func>(func), ptr(), tag() - 1);
    }

    /* Calls the function `func`, passing to it the pointer stored in this `TaggedPointer`,
    casted to the correct type, and returns the resulting value (with value category/cv-qualifiers
    preserved). `func` must have a single return type across all possible types pointed to by this
    `TaggedPointer`; if not, a compiler error will be raised. */
    template <typename Func>
    /* See the non-const overload of `call` for why the return type needs to be `decltype(auto)`. */
    decltype(auto) call(Func &&func) const {
        /* See the comment here in the non-const overload of `call`; these two functions are
        identical. */
        return detail::dispatch_call<Func, Ts...>(std::forward<Func>(func), ptr(), tag() - 1);
    }

    /* Two `TaggedPointer<Ts...>` are equal iff both their underlying pointer addresses and their
    tags are equal. */
    bool operator== (const TaggedPointer &other) const {
        return tagged_address == other.tagged_address;
    }

    /* Two `TaggedPointer<Ts...>` are unequal iff their underlying pointer addresses or their tags
    are unequal. */
    bool operator!= (const TaggedPointer &other) const {
        return tagged_address != other.tagged_address;
    }

    /* Constructs this `TaggedPointer` from the pointer to `T` `ptr`, where `T` is required to
    be one of the types specified in the parameter pack `Ts...`. */
    template <typename T>
    /* Use C++20 to require that `T` be equal to one of the types in `Ts...`. This check is
    done succinctly with `std::disjunction_v`, which is essentially an OR-statement. */
    requires std::disjunction_v<std::is_same<T, Ts>...>
    TaggedPointer(const T *ptr)
        /* The `TAG_SHIFT` least significant bits of the tagged address are the bits of the actual
        memory address of the given pointer `ptr`, while the remaining bits are used to encode the
        tag of the type `T`. In other words, the tagged address is found by taking the bitwise OR
        of the address `ptr` and the tag of `T` left-shifted by `TAG_SHIFT`. */
        : tagged_address{reinterpret_cast<uintptr_t>(ptr)
                         | (static_cast<uintptr_t>(get_tag_of_type<T>()) << TAG_SHIFT)}
    {}

    /* Observe that the constructor `TaggedPointer::TaggedPointer(const T *ptr)` will not accept
    the expression `nullptr` as an argument. This is because then `T` will deduce to
    `std::nullptr_t`, which should never be one of the types specified in the parameter pack
    `Ts...`. As a result, the `requires`-statement for that constructor, which mandates that
    `T` be equal to one of the types in `Ts...`, will fail, leading to a compilation error.

    To allow for constructing a `TaggedPointer` from `nullptr`, we simply introduce a constructor
    that accepts a `std::nullptr_t`. This constructor constructs a tagged null pointer; a tagged
    pointer with address 0. */
    TaggedPointer(std::nullptr_t) : tagged_address{0} {}

    /* The default constructor for `TaggedPointer` constructs a null pointer. That is,
    the internal address is set to 0s. This is equivalent to using the constructor
    `TaggedPointer::TaggedPointer(std::nullptr_t)`. */
    TaggedPointer() : tagged_address{0} {}
};