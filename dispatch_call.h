/* `detail::dispatch_call<Func, Ts...>(Func &&func, void *ptr, unsigned type_index)` calls `func`,
passing to it the pointer `ptr`, casted to the `type_index`th type in the parameter pack `Ts...`
(where `type_index` is zero-indexed). This is done by simply using a `switch`-statement on
`type_index`. */

namespace detail {

/* Handle parameter packs with exactly 1 type (nonconst version) */
template <typename Func, typename T>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    return func(static_cast<T*>(ptr));
}

/* Handle parameter packs with exactly 1 type (const version) */
template <typename Func, typename T>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    return func(static_cast<const T*>(ptr));
}

/* Handle parameter packs with exactly 2 types (nonconst version) */
template <typename Func, typename T0, typename T1>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        default: return func(static_cast<T1*>(ptr));
    }
}

/* Handle parameter packs with exactly 2 types (const version) */
template <typename Func, typename T0, typename T1>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        default: return func(static_cast<const T1*>(ptr));
    }
}

/* Handle parameter packs with exactly 3 types (nonconst version) */
template <typename Func, typename T0, typename T1, typename T2>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        default: return func(static_cast<T2*>(ptr));
    }
}

/* Handle parameter packs with exactly 3 types (const version) */
template <typename Func, typename T0, typename T1, typename T2>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        default: return func(static_cast<const T2*>(ptr));
    }
}

/* Handle parameter packs with exactly 4 types (nonconst version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        case 2: return func(static_cast<T2*>(ptr));
        default: return func(static_cast<T3*>(ptr));
    }
}

/* Handle parameter packs with exactly 4 types (const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        case 2: return func(static_cast<const T2*>(ptr));
        default: return func(static_cast<const T3*>(ptr));
    }
}

/* Handle parameter packs with exactly 5 types (nonconst version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3, typename T4>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        case 2: return func(static_cast<T2*>(ptr));
        case 3: return func(static_cast<T3*>(ptr));
        default: return func(static_cast<T4*>(ptr));
    }
}

/* Handle parameter packs with exactly 5 types (const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        case 2: return func(static_cast<const T2*>(ptr));
        case 3: return func(static_cast<const T3*>(ptr));
        default: return func(static_cast<const T4*>(ptr));
    }
}

/* Handle parameter packs with exactly 6 types (nonconst version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        case 2: return func(static_cast<T2*>(ptr));
        case 3: return func(static_cast<T3*>(ptr));
        case 4: return func(static_cast<T4*>(ptr));
        default: return func(static_cast<T5*>(ptr));
    }
}

/* Handle parameter packs with exactly 6 types (const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        case 2: return func(static_cast<const T2*>(ptr));
        case 3: return func(static_cast<const T3*>(ptr));
        case 4: return func(static_cast<const T4*>(ptr));
        default: return func(static_cast<const T5*>(ptr));
    }
}

/* Handle parameter packs with exactly 7 types (nonconst version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        case 2: return func(static_cast<T2*>(ptr));
        case 3: return func(static_cast<T3*>(ptr));
        case 4: return func(static_cast<T4*>(ptr));
        case 5: return func(static_cast<T5*>(ptr));
        default: return func(static_cast<T6*>(ptr));
    }
}

/* Handle parameter packs with exactly 7 types (const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        case 2: return func(static_cast<const T2*>(ptr));
        case 3: return func(static_cast<const T3*>(ptr));
        case 4: return func(static_cast<const T4*>(ptr));
        case 5: return func(static_cast<const T5*>(ptr));
        default: return func(static_cast<const T6*>(ptr));
    }
}

/* Handle parameter packs with exactly 8 types (nonconst version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        case 2: return func(static_cast<T2*>(ptr));
        case 3: return func(static_cast<T3*>(ptr));
        case 4: return func(static_cast<T4*>(ptr));
        case 5: return func(static_cast<T5*>(ptr));
        case 6: return func(static_cast<T6*>(ptr));
        default: return func(static_cast<T7*>(ptr));
    }
}

/* Handle parameter packs with exactly 8 types (const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        case 2: return func(static_cast<const T2*>(ptr));
        case 3: return func(static_cast<const T3*>(ptr));
        case 4: return func(static_cast<const T4*>(ptr));
        case 5: return func(static_cast<const T5*>(ptr));
        case 6: return func(static_cast<const T6*>(ptr));
        default: return func(static_cast<const T7*>(ptr));
    }
}

/* Handle parameter packs with more than 8 types (non-const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7, typename... Ts>
auto dispatch_call(Func &&func, void *ptr, unsigned type_index) {
    /* Check if `type_index` corresponds to any of the first 8 types. If it does, then
    we are done. Otherwise, if `type_index` does not correspond to any of the first 8
    remaining types (if `type_index` >= 8), then we make a recursive call to `dispatch_call`
    with the remaining types. */
    switch(type_index) {
        case 0: return func(static_cast<T0*>(ptr));
        case 1: return func(static_cast<T1*>(ptr));
        case 2: return func(static_cast<T2*>(ptr));
        case 3: return func(static_cast<T3*>(ptr));
        case 4: return func(static_cast<T4*>(ptr));
        case 5: return func(static_cast<T5*>(ptr));
        case 6: return func(static_cast<T6*>(ptr));
        case 7: return func(static_cast<T7*>(ptr));
        default:
            /* We need to `std::forward` `func` for the same reason as we needed to in
            `TaggedPointer::call()`; see the comments there */
            return dispatch_call<Func, Ts...>(std::forward<Func>(func), ptr, type_index - 8);
    }
}

/* Handle parameter packs with more than 8 types (const version) */
template <typename Func, typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7, typename... Ts>
auto dispatch_call(Func &&func, const void *ptr, unsigned type_index) {
    /* Check if `type_index` corresponds to any of the first 8 types. If it does, then
    we are done. Otherwise, if `type_index` does not correspond to any of the first 8
    remaining types (if `type_index` >= 8), then we make a recursive call to `dispatch_call`
    with the remaining types. */
    switch(type_index) {
        case 0: return func(static_cast<const T0*>(ptr));
        case 1: return func(static_cast<const T1*>(ptr));
        case 2: return func(static_cast<const T2*>(ptr));
        case 3: return func(static_cast<const T3*>(ptr));
        case 4: return func(static_cast<const T4*>(ptr));
        case 5: return func(static_cast<const T5*>(ptr));
        case 6: return func(static_cast<const T6*>(ptr));
        case 7: return func(static_cast<const T7*>(ptr));
        default:
            /* We need to `std::forward` `func` for the same reason as we needed to in
            `TaggedPointer::call()`; see the comments there */
            return dispatch_call<Func, Ts...>(std::forward<Func>(func), ptr, type_index - 8);
    }
}

};  /* Ending bracket for `namespace detail` */