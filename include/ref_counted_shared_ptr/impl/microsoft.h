#ifndef REF_COUNTED_SHARED_PTR_MICROSOFT_H_
#define REF_COUNTED_SHARED_PTR_MICROSOFT_H_

#include <memory>
#include <type_traits>

#include "ref_counted_shared_ptr/detail/access_private_member.h"

#define REF_COUNTED_SHARED_PTR_STD WINDOWS
#define REF_COUNTED_SHARED_PTR_STD_WINDOWS
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(...)             \
template struct ref_counted_shared_ptr::detail::make_private_member<         \
    ::ref_counted_shared_ptr::detail::std::microsoft::_wptr< __VA_ARGS__ >,  \
    &::std::enable_shared_from_this< __VA_ARGS__ >::_Wptr                    \
>;                                                                           \
                                                                             \
template struct ref_counted_shared_ptr::detail::make_private_member<         \
    ::ref_counted_shared_ptr::detail::std::microsoft::_rep< __VA_ARGS__ >,   \
    &::std::_Ptr_base< __VA_ARGS__ >::_Rep                                   \
>;                                                                           \
                                                                             \
template<>                                                                   \
struct ref_counted_shared_ptr::detail::std::microsoft::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"


namespace ref_counted_shared_ptr {
namespace detail {
namespace std {
namespace microsoft {

template<typename T>
struct _wptr : private_member<_wptr<T>, ::std::enable_shared_from_this<T>, ::std::weak_ptr<T>> {};
template<typename T>
struct _rep : private_member<_rep<T>, ::std::_Ptr_base<T>, ::std::_Ref_count_base*> {};
struct _uses : private_member<_uses, ::std::_Ref_count_base, ::std::_Atomic_counter_t> {};

template<typename T>
struct defined_private_accessors : ::std::false_type {};

}
}
}
}

namespace ref_counted_shared_ptr {
namespace detail {

template struct make_private_member<std::microsoft::_uses, &::std::_Ref_count_base::_Uses>;

}
}

namespace ref_counted_shared_ptr {
namespace detail {
namespace std {

struct implementation_information {
    template<typename T> using shared_ptr = ::std::shared_ptr<T>;
    template<typename T> using weak_ptr = ::std::weak_ptr<T>;
    template<typename T> using enable_shared_from_this = ::std::enable_shared_from_this<T>;

    using control_block_type = ::std::_Ref_count_base;
    using atomic_count_type = ::std::_Atomic_counter_t;
    using regular_count_type = long;

    template<typename T>
    static weak_ptr<T>& get_weak_ptr(const enable_shared_from_this<T>& p) noexcept {
        static_assert(
            ::ref_counted_shared_ptr::detail::std::microsoft::defined_private_accessors<T>::value,
            "std::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()"
        );

        return const_cast<::std::weak_ptr<T>&>(p.*::ref_counted_shared_ptr::detail::std::microsoft::_wptr<T>::get_value());
    }

    template<typename T>
    static control_block_type*& get_control_block(weak_ptr<T>& p) noexcept {
        return p.*::ref_counted_shared_ptr::detail::std::microsoft::_rep<T>::get_value();
    }

    static atomic_count_type& get_count(control_block_type& control_block) noexcept {
        return control_block.*::ref_counted_shared_ptr::detail::std::microsoft::_uses::get_value();
    }

    static long cast_count_to_long(regular_count_type count) {
        return count;
    }

    static long get_use_count(control_block_type& control_block) noexcept {
        return control_block._Use_count();
    }

    static regular_count_type increment_and_fetch(atomic_count_type& count, control_block_type&) noexcept {
        return _MT_INCR(count);
    }

    static regular_count_type decrement_and_fetch(atomic_count_type& count, control_block_type&) noexcept {
        return _MT_DECR(count);
    }

    static void on_zero_references(atomic_count_type&, control_block_type& control_block) noexcept {
        control_block._Incref();
        control_block._Decref();
    }
};

}
}
}

#endif  // REF_COUNTED_SHARED_PTR_MICROSOFT_H_
