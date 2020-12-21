#ifndef REF_COUNTED_SHARED_PTR_LIBSTDCXX_H_
#define REF_COUNTED_SHARED_PTR_LIBSTDCXX_H_

#include <memory>
#include <type_traits>

#include "ref_counted_shared_ptr/detail/access_private_member.h"


#define REF_COUNTED_SHARED_PTR_STD LIBSTDCXX
#define REF_COUNTED_SHARED_PTR_STD_LIBSTDCXX
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(...)                    \
template struct ref_counted_shared_ptr::detail::make_private_member<                \
    ::ref_counted_shared_ptr::detail::std::libstdcxx::_m_weak_this< __VA_ARGS__ >,  \
    &::std::enable_shared_from_this< __VA_ARGS__ >::_M_weak_this                    \
>;                                                                                  \
                                                                                    \
template struct ref_counted_shared_ptr::detail::make_private_member<                \
    ::ref_counted_shared_ptr::detail::std::libstdcxx::_m_refcount< __VA_ARGS__ >,   \
    &::std::__weak_ptr< __VA_ARGS__ >::_M_refcount                                  \
>;                                                                                  \
                                                                                    \
template<> struct ref_counted_shared_ptr::detail::std::libstdcxx::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"


namespace ref_counted_shared_ptr {
namespace detail {
namespace std {
namespace libstdcxx {

template<typename T>
struct _m_weak_this : private_member<_m_weak_this<T>, ::std::enable_shared_from_this<T>, ::std::weak_ptr<T>> {};
template<typename T>
struct _m_refcount : private_member<_m_refcount<T>, ::std::__weak_ptr<T>, ::std::__weak_count<>> {};
struct _m_pi : private_member<_m_pi, ::std::__weak_count<>, ::std::_Sp_counted_base<>*> {};
struct _m_use_count : private_member<_m_use_count, ::std::_Sp_counted_base<>, ::_Atomic_word> {};

template<typename T>
struct defined_private_accessors : ::std::false_type {};

}
}
}
}

namespace ref_counted_shared_ptr {
namespace detail {

template struct make_private_member<std::libstdcxx::_m_pi, &::std::__weak_count<>::_M_pi>;
template struct make_private_member<std::libstdcxx::_m_use_count, &::std::_Sp_counted_base<>::_M_use_count>;

}
}

namespace ref_counted_shared_ptr {
namespace detail {
namespace std {

struct implementation_information {
    template<typename T> using shared_ptr = ::std::shared_ptr<T>;
    template<typename T> using weak_ptr = ::std::weak_ptr<T>;
    template<typename T> using enable_shared_from_this = ::std::enable_shared_from_this<T>;

    using control_block_type = ::std::_Sp_counted_base<>;
    using atomic_count_type = ::_Atomic_word;
    using regular_count_type = long;

    template<typename T>
    static weak_ptr<T>& get_weak_ptr(const enable_shared_from_this<T>& p) noexcept {
        static_assert(
            ::ref_counted_shared_ptr::detail::std::libstdcxx::defined_private_accessors<T>::value,
            "std::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()"
        );

        return const_cast<::std::weak_ptr<T>&>(p.*::ref_counted_shared_ptr::detail::std::libstdcxx::_m_weak_this<T>::get_value());
    }

    template<typename T>
    static control_block_type*& get_control_block(weak_ptr<T>& p) noexcept {
        return p.*::ref_counted_shared_ptr::detail::std::libstdcxx::_m_refcount<T>::get_value().*libstdcxx::_m_pi::get_value();
    }

    static atomic_count_type& get_count(control_block_type& control_block) noexcept {
        return control_block.*::ref_counted_shared_ptr::detail::std::libstdcxx::_m_use_count::get_value();
    }

    static long cast_count_to_long(regular_count_type count) {
        return count;
    }

    static long get_use_count(control_block_type& control_block) noexcept {
        return control_block._M_get_use_count();
    }

    static regular_count_type increment_and_fetch(atomic_count_type& count, control_block_type&) noexcept {
        return ::__gnu_cxx::__exchange_and_add(&count, +1) + 1;
    }

    static regular_count_type decrement_and_fetch(atomic_count_type& count, control_block_type&) noexcept {
        return ::__gnu_cxx::__exchange_and_add(&count, -1) - 1;
    }

    static void on_zero_references(atomic_count_type&, control_block_type& control_block) noexcept {
        control_block._M_add_ref_copy();
        control_block._M_release();
    }
};

}
}
}

#endif  // REF_COUNTED_SHARED_PTR_LIBSTDCXX_H_
