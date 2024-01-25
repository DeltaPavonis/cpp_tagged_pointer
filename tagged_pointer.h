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

/* `ContainsType<T, Ts...>` is satisfied iff `T` is one of the types in the parameter pack
`Ts...`. */
template <typename T, typename... Ts>
concept ContainsType = (std::is_same_v<T, Ts> || ...);

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
    static constexpr auto num_types() {return sizeof...(Ts);}

    /* Returns the tag of the type `T` in this `TaggedPointer<Ts...>`. The tag of a type `T`
    is determined as follows:
    - If `T` is `std::nullptr_t` (that is, if this `TaggedPointer` was constructed or set
    equal to `nullptr`), then its tag is 0.
    - Otherwise, the tag of `T` equals its ONE-indexed position within the parameter pack
    `Ts...`.

    Thus, the possible tags fall in the range `[0, num_types()]`. */
    template <typename T>
    requires detail::ContainsType<T, std::nullptr_t, Ts...>
    static constexpr unsigned get_tag_of_type() {
        /* If `T` is `std::nullptr_t`, then its tag is defined to be 0 (this means a
        `TaggedPointer` constructed with `nullptr` will have tag equal to 0). Otherwise,
        the tag of `T` is equivalent to its one-indexed position within `Ts...`. As a
        result, the tag of `T` is equivalent to its ZERO-indexed position within
        `std::nullptr_t, Ts...`. */
        return detail::IndexOfType_v<T, std::nullptr_t, Ts...>;
    }

    /* Returns the current tag of this `TaggedPointer`. */
    auto tag() const {return static_cast<unsigned>(tagged_address >> TAG_SHIFT);}
    
    /* Returns the address of the pointer stored in this `TaggedPointer` as a `void*`. */
    const void *ptr() const {return reinterpret_cast<const void*>(tagged_address & GET_PTR_MASK);}
    /* Returns the address of the pointer stored in this `TaggedPointer` as a `void*`. */
    void *ptr() {return reinterpret_cast<void*>(tagged_address & GET_PTR_MASK);}

    /* Returns the pointer stored in this `TaggedPointer` casted to a `const T*` if the current
    type pointed to by this `TaggedPointer` is the same as `T`, and returns `nullptr` otherwise.
    
    To bypass the type equality check between `T` and the true current type pointed to by this
    `TaggedPointer`, use `cast_unchecked()` instead. */
    template <typename T>
    requires detail::ContainsType<T, Ts...>
    const T *cast() const {
        /* See the comments inside `const T *cast_unchecked()` as to why we should not
        directly `reinterpret_cast` the untagged memory address directly to a `T*`. */
        return points_to_type<T>() ? static_cast<const T*>(ptr()) : nullptr;
    }

    /* Returns the pointer stored in this `TaggedPointer` casted to a `T*` if the current
    type pointed to by this `TaggedPointer` is the same as `T`, and returns `nullptr` otherwise.
    
    To bypass the type equality check between `T` and the true current type pointed to by this
    `TaggedPointer`, use `cast_unchecked()` instead. */
    template <typename T>
    requires detail::ContainsType<T, Ts...>
    T *cast() {
        /* See the comments inside `const T *cast_unchecked()` as to why we should not
        directly `reinterpret_cast` the untagged memory address directly to a `T*`. */
        return points_to_type<T>() ? static_cast<T*>(ptr()) : nullptr;
    }

    /* Returns the pointer stored in this `TaggedPointer` casted to a `const T*`. Does not perform
    any check that this `TaggedPointer` truly points to an object of type `T`. */ 
    template <typename T>
    requires detail::ContainsType<T, Ts...>
    const T *cast_unchecked() const {
        /* Note that we cannot just take the untagged memory address and `reinterpret_cast` it
        directly to a `T*`, because the untagged memory address was computed by first
        `static_cast`ing the original `T*` to a `void*`, then `reinterpret_cast`ing that
        `void*` to a `uintptr_t` (and this is because `uintptr_t` is only guaranteed by the
        C standard to be able to represent any `void*`, not necessarily any `T*`; there is
        no requirement that the bit pattern for `T*` and `void*` have the same representation
        (and it won't be, on systems that are not byte-addressable). Thus, we need to do
        `T*` -> `void*` -> `uintptr_t`). Now, we need to reverse this process: we will first
        `reinterpret_cast` the memory address back into a `void*` (done inside the call
        to `ptr()`), then `static_cast` that `void*` back to a `T*`. The reason why we
        can't just `reinterpret_cast` the untagged memory address to a `T*` is, again,
        because `T*` and `void*` are not necessarily represented the same way (and
        `reinterpret_cast` only guarantees that casting the `void*` to another type
        (`uintptr_t` in this case) and then casting the `uintptr_t` back to a `void*` yields
        the original `void*`; this, added to the fact that `void*` and `T*` may have
        different bit-level representations, means directly `reinterpret_casting` the `uintptr_t`
        to a `T*` is unsafe). Thus, we need to do `uintptr_t` -> `void*` -> `T*` in order to
        guarantee that the returned pointer is correct.
        
        Source: http://tinyurl.com/3spmuxc6 */
        return static_cast<const T*>(ptr());
    }

    /* Returns the pointer stored in this `TaggedPointer` casted to a `T*`. Does not perform any
    check that this `TaggedPointer` truly points to an object of type `T`. */ 
    template <typename T>
    requires detail::ContainsType<T, Ts...>
    T *cast_unchecked() {
        /* See the comments inside `const T *cast_unchecked()` as to why we should not
        directly `reinterpret_cast` the untagged memory address directly to a `T*`. */
        return static_cast<T*>(ptr());
    }

    /* Returns `true` iff `T` is the type currently pointed to by this `TaggedPointer`. */
    template <typename T>
    requires detail::ContainsType<T, Ts...>
    bool points_to_type() const {
        /* Simply check if the current tag of this `TaggedPointer` matches the tag
        corresponding to the type `T`. */
        return tag() == get_tag_of_type<T>();
    }

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

    /* Constructs this `TaggedPointer` from `ptr`, a pointer to `T`. `T` is required to
    be one of the types in the parameter pack `Ts...`. */
    template <typename T>
    /* Use C++20 to require that `T` be equal to one of the types in `Ts...`. This check is
    done succinctly with C++20 concepts.  */
    requires detail::ContainsType<T, Ts...>
    TaggedPointer(const T *ptr)
        /* The `TAG_SHIFT` least significant bits of the tagged address are the bits of the actual
        memory address of the given pointer `ptr`, while the remaining bits are used to encode the
        tag of the type `T`. In other words, the tagged address is found by taking the bitwise OR
        of the address `ptr` and the tag of `T` left-shifted by `TAG_SHIFT`.
        
        Note that we first `static_cast` `ptr` to a `const void*` before we `reinterpret_cast` it
        to a `uintptr_t`. This is because `uintptr_t` is only guaranteed to be able to hold a
        `void*`, and not a general `T*`. Note that this means in order to turn the `uintptr_t`
        back into a `T*`, we need to first `reinterpret_cast` it to a `void*` THEN `static_cast`
        it back to a `T*`. See the comments in `const T *cast_unchecked()`. */
        : tagged_address{reinterpret_cast<uintptr_t>(static_cast<const void*>(ptr))
                       | (static_cast<uintptr_t>(get_tag_of_type<T>()) << TAG_SHIFT)}
    {}

    /* Observe that the constructor `TaggedPointer::TaggedPointer(const T *ptr)` will not accept
    the expression `nullptr` as an argument. This is because then `T` will deduce to
    `std::nullptr_t`, which probably isn't one of the types specified in the parameter pack
    `Ts...`. As a result, the `requires`-statement for that constructor, which mandates that
    `T` be equal to one of the types in `Ts...`, will fail, leading to a compilation error.

    To allow for constructing a `TaggedPointer` from `nullptr`, we simply introduce a constructor
    that accepts a `std::nullptr_t`. */
    TaggedPointer(std::nullptr_t)
        /* The tag of `nullptr` is defined to be 0 (see `get_tag_of_type`), and `nullptr` is
        guaranteed to `reinterpret_cast` to 0 (see http://tinyurl.com/yh5my6kc). Thus, the
        tagged address of a tagged null pointer is 0. */
        : tagged_address{0}
    {}

    /* The default constructor for `TaggedPointer` constructs a tagged null pointer. */
    TaggedPointer() : TaggedPointer(nullptr) {}
};