#ifndef REF_COUNTED_SHARED_PTR_LIBCXX_H_
#define REF_COUNTED_SHARED_PTR_LIBCXX_H_

#include <memory>
#include <type_traits>
#include <__config>

#include "ref_counted_shared_ptr/detail/access_private_member.h"

#define REF_COUNTED_SHARED_PTR_STD LIBCXX
#define REF_COUNTED_SHARED_PTR_STD_LIBCXX
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(...)                    \
template struct ref_counted_shared_ptr::detail::make_private_member<                \
    ::ref_counted_shared_ptr::detail::std::libcxx::_weak_this_< __VA_ARGS__ >,      \
    &::std::enable_shared_from_this< __VA_ARGS__ >::__weak_this_                    \
>;                                                                                  \
                                                                                    \
template struct ref_counted_shared_ptr::detail::make_private_member<                \
    ::ref_counted_shared_ptr::detail::std::libcxx::_cntrl_< __VA_ARGS__ >,          \
    &::std::weak_ptr< __VA_ARGS__ >::__cntrl_                                       \
>;                                                                                  \
                                                                                    \
template<> struct ref_counted_shared_ptr::detail::std::libcxx::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"


namespace ref_counted_shared_ptr {
namespace detail {
namespace std {
namespace libcxx {

template<typename T>
struct _weak_this_ : private_member<_weak_this_<T>, ::std::enable_shared_from_this<T>, ::std::weak_ptr<T>> {};
template<typename T>
struct _cntrl_ : private_member<_cntrl_<T>, ::std::weak_ptr<T>, ::std::__shared_weak_count*> {};

static_assert(::std::is_base_of<::std::__shared_count, ::std::__shared_weak_count>::value, "libc++ implementation of weak_ptr incompatible with what is expected by ref_counted_shared_ptr");
struct _shared_owners_ : private_member<_shared_owners_, ::std::__shared_count, long> {};

inline void _noexcept_function_type() noexcept;
struct _on_zero_shared : private_member<_on_zero_shared, ::std::__shared_count, decltype(_noexcept_function_type)> {};

template<typename T>
struct defined_private_accessors : ::std::false_type {};

}
}
}
}

namespace ref_counted_shared_ptr {
namespace detail {

template struct make_private_member<std::libcxx::_shared_owners_, &::std::__shared_count::__shared_owners_>;
template struct make_private_member<std::libcxx::_on_zero_shared, &::std::__shared_count::__on_zero_shared>;

}
}

namespace ref_counted_shared_ptr {
namespace detail {
namespace std {

struct implementation_information {
    template<typename T> using shared_ptr = ::std::shared_ptr<T>;
    template<typename T> using weak_ptr = ::std::weak_ptr<T>;
    template<typename T> using enable_shared_from_this = ::std::enable_shared_from_this<T>;

    using control_block_type = ::std::__shared_weak_count;
    using atomic_count_type = long;
    using regular_count_type = long;

    static ::std::__shared_count& upcast_control_block(::std::__shared_weak_count& control_block) noexcept {
        // Cast to private base via C-style cast, so disable those warnings
#if defined(_LIBCPP_COMPILER_CLANG) || defined(_LIBCPP_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
        return (::std::__shared_count&) control_block;
#if defined(_LIBCPP_COMPILER_CLANG) || defined(_LIBCPP_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
    }

    template<typename T>
    static weak_ptr<T>& get_weak_ptr(const enable_shared_from_this<T>& p) noexcept {
        static_assert(
            ::ref_counted_shared_ptr::detail::std::libcxx::defined_private_accessors<T>::value,
            "std::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()"
        );

        return const_cast<::std::weak_ptr<T>&>(p.*::ref_counted_shared_ptr::detail::std::libcxx::_weak_this_<T>::get_value());
    }

    template<typename T>
    static control_block_type*& get_control_block(weak_ptr<T>& p) noexcept {
        return p.*::ref_counted_shared_ptr::detail::std::libcxx::_cntrl_<T>::get_value();
    }

    static atomic_count_type& get_count(control_block_type& control_block) noexcept {
        return upcast_control_block(control_block).*::ref_counted_shared_ptr::detail::std::libcxx::_shared_owners_::get_value();
    }

    static long cast_count_to_long(regular_count_type count) {
        return count + 1;
    }

    static long get_use_count(control_block_type& control_block) noexcept {
        return control_block.use_count();
    }

    static regular_count_type increment_and_fetch(atomic_count_type& count, control_block_type&) noexcept {
        return ::std::__libcpp_atomic_refcount_increment(count);
    }

    static regular_count_type decrement_and_fetch(atomic_count_type& count, control_block_type&) noexcept {
        return ::std::__libcpp_atomic_refcount_decrement(count);
    }

    static void on_zero_references(atomic_count_type&, control_block_type& control_block) noexcept {
        (upcast_control_block(control_block).*::ref_counted_shared_ptr::detail::std::libcxx::_on_zero_shared::get_value())();
    }
};

}
}
}

#endif  // REF_COUNTED_SHARED_PTR_LIBCXX_H_
