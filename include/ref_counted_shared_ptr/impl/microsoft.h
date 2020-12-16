#ifndef REF_COUNTED_SHARED_PTR_MICROSOFT_H_
#define REF_COUNTED_SHARED_PTR_MICROSOFT_H_

#include <memory>
#include <type_traits>

#include "ref_counted_shared_ptr/detail/access_private_member.h"

#define REF_COUNTED_SHARED_PTR_STD WINDOWS
#define REF_COUNTED_SHARED_PTR_STD_WINDOWS
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(...)             \
template struct ::ref_counted_shared_ptr::detail::make_private_member<       \
    ::ref_counted_shared_ptr::detail::std::microsoft::_wptr< __VA_ARGS__ >,  \
    &::std::enable_shared_from_this< __VA_ARGS__ >::_Wptr                    \
>;                                                                           \
                                                                             \
template struct ::ref_counted_shared_ptr::detail::make_private_member<       \
    ::ref_counted_shared_ptr::detail::std::microsoft::_rep< __VA_ARGS__ >,   \
    &::std::_Ptr_base< __VA_ARGS__ >::_Rep                                   \
>;                                                                           \
                                                                             \
template<>                                                                   \
struct ::ref_counted_shared_ptr::detail::std::microsoft::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"

namespace ref_counted_shared_ptr {
namespace detail {
namespace std {
namespace microsoft {  // v ref_counted_shared_ptr::detail::std::microsoft

struct _uses : private_member<_uses, ::std::_Ref_count_base, ::std::_Atomic_counter_t> {};

}
}  // ^ ref_counted_shared_ptr::detail::std::microsoft, v ref_counted_shared_ptr::detail

template struct make_private_member<std::microsoft::_uses, &::std::_Ref_count_base::_Uses>;

}
}  // ^ ref_counted_shared_ptr::detail, v ::


namespace ref_counted_shared_ptr {
namespace detail {
namespace std {
namespace microsoft {  // v ref_counted_shared_ptr::detail::std::microsoft

template<typename T>
struct _wptr : private_member<_wptr<T>, ::std::enable_shared_from_this<T>, ::std::weak_ptr<T>> {};
template<typename T>
struct _rep : private_member<_rep<T>, ::std::_Ptr_base<T>, ::std::_Ref_count_base*> {};
template<typename T>
struct defined_private_accessors : ::std::false_type {};

template<typename T>
::std::weak_ptr<T>& get_weak_ptr(const ::std::weak_ptr<T>& p) noexcept {
    static_assert(defined_private_accessors<T>::value, "std::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()");

    // const_cast is fine since _Wptr is mutable
    return const_cast<::std::weak_ptr<T>&>(p.*_wptr<T>::get_value());
}
template<typename T>
::std::_Ref_count_base*& get_control_block(const ::std::enable_shared_from_this<T>& p) noexcept {
    return get_weak_ptr(p).*_rep<T>::get_value();
}

inline ::std::_Atomic_counter_t& get_count(::std::_Ref_count_base& control_block) noexcept {
    return control_block.*_uses::get_value();
}

}
}
}
}  // ^ ref_counted_shared_ptr::detail::std::microsoft, v ::

namespace ref_counted_shared_ptr {
namespace std {

template<typename Self>
struct ref_counted_shared_ptr : ::std::enable_shared_from_this<Self> {
    constexpr ref_counted_shared_ptr() noexcept = default;
    ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;

    ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept = default;

    long incref() const {
        crtp_checks();

        ::std::_Ref_count_base*& control_block = ::ref_counted_shared_ptr::detail::std::microsoft::get_control_block(*this);
        if (control_block) {
            return _MT_INCR(::ref_counted_shared_ptr::detail::std::microsoft::get_count(*control_block));
        }

        // First time. Create control block
        ::std::shared_ptr<const Self> s = this->shared_from_this();  // May throw
        return _MT_INCR(::ref_counted_shared_ptr::detail::std::microsoft::get_count(*control_block)) - 1;  // -1 for s
    }

    long decref() const noexcept {
        crtp_checks();

        // Must have control block to call decref
        ::std::_Ref_count_base& control_block = *::ref_counted_shared_ptr::detail::std::microsoft::get_control_block(*this);
        ::std::_Atomic_counter_t& count = ::ref_counted_shared_ptr::detail::std::microsoft::get_count(control_block);
        long new_count = _MT_DECR(count);
        if (new_count != 0) return new_count;

        // *this must be the only reference, so no race condition
        _MT_INCR(count);
        control_block._Decref();
        return 0;
    }

    long use_count() const noexcept {
        crtp_checks();
        ::std::_Ref_count_base* control_block = ::ref_counted_shared_ptr::detail::std::microsoft::get_control_block(*this);
        if (!control_block) return 0;
        return reinterpret_cast<volatile long&>(::ref_counted_shared_ptr::detail::std::microsoft::get_count(*control_block));
    }

    ::std::weak_ptr<T> weak_from_this() noexcept {
        return ::ref_counted_shared_ptr::detail::std::microsoft::get_weak_ptr(*this);
    }
    ::std::weak_ptr<const T> weak_from_this() const noexcept {
        return ::ref_counted_shared_ptr::detail::std::microsoft::get_weak_ptr(*this);
    }
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value, "std::ref_counted_shared_ptr<Self>: Self must derive from std::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_MICROSOFT_H_
