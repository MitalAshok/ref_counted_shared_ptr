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

template struct make_private_member<std::libstdcxx::_m_pi, &::std::__weak_count<>::_M_pi>;
template struct make_private_member<std::libstdcxx::_m_use_count, &::std::_Sp_counted_base<>::_M_use_count>;

namespace std {
namespace libstdcxx {

template<typename T>
::std::weak_ptr<T>& get_weak_ptr(const ::std::enable_shared_from_this<T>& p) noexcept {
    static_assert(defined_private_accessors<T>::value, "std::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()");

    return const_cast<::std::weak_ptr<T>&>(p.*_m_weak_this<T>::get_value());
}

template<typename T>
::std::_Sp_counted_base<>*& get_control_block(const ::std::enable_shared_from_this<T>& p) noexcept {
    return get_weak_ptr(p).*_m_refcount<T>::get_value().*_m_pi::get_value();
}

inline ::_Atomic_word& get_count(::std::_Sp_counted_base<>& control_block) noexcept {
    return control_block.*_m_use_count::get_value();
}

}
}

}
}

namespace ref_counted_shared_ptr {
namespace std {

template<typename Self>
struct ref_counted_shared_ptr : ::std::enable_shared_from_this<Self> {
protected:
    constexpr ref_counted_shared_ptr() noexcept = default;
    ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;

    ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept = default;

    ~ref_counted_shared_ptr() = default;

    long incref() const {
        crtp_checks();

        ::std::_Sp_counted_base<>*& control_block = ::ref_counted_shared_ptr::detail::std::libstdcxx::get_control_block(*this);
        if (control_block) {
            ::_Atomic_word& count = ::ref_counted_shared_ptr::detail::std::libstdcxx::get_count(*control_block);
            long old_count = __gnu_cxx::__exchange_and_add(&count, +1);
            return old_count + 1;
        }

        static_cast<void>(::std::shared_ptr<const Self>(::std::weak_ptr<const Self>()));  // Throws bad_weak_ptr
        return incref();
    }

    long decref() const noexcept {
        crtp_checks();

        // Must have control block to call decref
        ::std::_Sp_counted_base<>& control_block = *::ref_counted_shared_ptr::detail::std::libstdcxx::get_control_block(*this);
        ::_Atomic_word& count = ::ref_counted_shared_ptr::detail::std::libstdcxx::get_count(control_block);
        long old_count = __gnu_cxx::__exchange_and_add(&count, -1);
        if (old_count != 1) return old_count - 1;

        // *this must be the only reference, so no race condition
        control_block._M_add_ref_copy();
        control_block._M_release();
        return 0;
    }

    long use_count() const noexcept {
        crtp_checks();
        ::std::_Sp_counted_base<>* control_block = ::ref_counted_shared_ptr::detail::std::libstdcxx::get_control_block(*this);
        if (!control_block) return 0;
        return __atomic_load_n(&::ref_counted_shared_ptr::detail::std::libstdcxx::get_count(*control_block), __ATOMIC_RELAXED);
    }

public:
    ::std::weak_ptr<Self> weak_from_this() noexcept {
        return ::ref_counted_shared_ptr::detail::std::libstdcxx::get_weak_ptr(*this);
    }
    ::std::weak_ptr<const Self> weak_from_this() const noexcept {
        return ::ref_counted_shared_ptr::detail::std::libstdcxx::get_weak_ptr(*this);
    }
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value, "std::ref_counted_shared_ptr<Self>: Self must derive from std::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_LIBSTDCXX_H_
