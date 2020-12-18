#ifndef REF_COUNTED_SHARED_PTR_LIBCXX_H_
#define REF_COUNTED_SHARED_PTR_LIBCXX_H_

#include <memory>
#include <type_traits>
#include <__config>

#include "ref_counted_shared_ptr/detail/access_private_member.h"

#define REF_COUNTED_SHARED_PTR_STD LIBCXX
#define REF_COUNTED_SHARED_PTR_STD_LIBCXX
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(...)                    \
template struct ::ref_counted_shared_ptr::detail::make_private_member<              \
    ::ref_counted_shared_ptr::detail::std::libcxx::_weak_this_< __VA_ARGS__ >,      \
    &::std::enable_shared_from_this< __VA_ARGS__ >::__weak_this_                    \
>;                                                                                  \
                                                                                    \
template struct ::ref_counted_shared_ptr::detail::make_private_member<              \
    ::ref_counted_shared_ptr::detail::std::libcxx::_cntrl_< __VA_ARGS__ >,          \
    &::std::weak_ptr< __VA_ARGS__ >::__cntrl_                                       \
>;                                                                                  \
                                                                                    \
template<> struct ::ref_counted_shared_ptr::detail::std::libcxx::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

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

template struct make_private_member<std::libcxx::_shared_owners_, &::std::__shared_count::__shared_owners_>;
template struct make_private_member<std::libcxx::_on_zero_shared, &::std::__shared_count::__on_zero_shared>;

namespace std {
namespace libcxx {

template<typename T>
::std::weak_ptr<T>& get_weak_ptr(const ::std::enable_shared_from_this<T>& p) noexcept {
    static_assert(defined_private_accessors<T>::value, "std::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()");

    return const_cast<::std::weak_ptr<T>&>(p.*_weak_this_<T>::get_value());
}

template<typename T>
::std::__shared_weak_count*& get_control_block(const ::std::enable_shared_from_this<T>& p) noexcept {
    return get_weak_ptr(p).*_cntrl_<T>::get_value();
}

// Cast to private base via C-style cast, so disable those warnings
#if defined(_LIBCPP_COMPILER_CLANG) || defined(_LIBCPP_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
inline long& get_count(::std::__shared_weak_count& control_block) noexcept {
    return ((::std::__shared_count&) control_block).*_shared_owners_::get_value();
}

inline void on_zero_shared(::std::__shared_weak_count& control_block) noexcept {
    (((::std::__shared_count&) control_block).*_on_zero_shared::get_value())();
}
#if defined(_LIBCPP_COMPILER_CLANG) || defined(_LIBCPP_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

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

        ::std::__shared_weak_count*& control_block = ::ref_counted_shared_ptr::detail::std::libcxx::get_control_block(*this);
        if (control_block) {
            long& count = ::ref_counted_shared_ptr::detail::std::libcxx::get_count(*control_block);
            long new_count = ::std::__libcpp_atomic_refcount_increment(count);
            return new_count + 1;
        }

        // First time. Create control block
        ::std::shared_ptr<const Self> s = this->shared_from_this();  // May throw
        long& count = ::ref_counted_shared_ptr::detail::std::libcxx::get_count(*control_block);
        long new_count = ::std::__libcpp_atomic_refcount_increment(count);
        return new_count;  // No +1 for s
    }

    long decref() const noexcept {
        crtp_checks();

        // Must have control block to call decref
        ::std::__shared_weak_count& control_block = *::ref_counted_shared_ptr::detail::std::libcxx::get_control_block(*this);
        long& count = ::ref_counted_shared_ptr::detail::std::libcxx::get_count(control_block);
        // libc++'s __shared_owners_ is offset by -1 (since it doesn't count +1 for weak pointers)
        // so the new value after decrement still needs to add this one.
        long new_count = ::std::__libcpp_atomic_refcount_decrement(count);
        if (new_count != -1) return new_count + 1;

        // Destroy shared object (*this) and deallocate if possible
        ::ref_counted_shared_ptr::detail::std::libcxx::on_zero_shared(control_block);
        return 0;
    }

    long use_count() const noexcept {
        crtp_checks();
        ::std::__shared_weak_count* control_block = ::ref_counted_shared_ptr::detail::std::libcxx::get_control_block(*this);
        if (!control_block) return 0;
        return control_block->use_count();
    }

    ::std::weak_ptr<Self> weak_from_this() noexcept {
        return ::ref_counted_shared_ptr::detail::std::libcxx::get_weak_ptr(*this);
    }
    ::std::weak_ptr<const Self> weak_from_this() const noexcept {
        return ::ref_counted_shared_ptr::detail::std::libcxx::get_weak_ptr(*this);
    }
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value, "std::ref_counted_shared_ptr<Self>: Self must derive from std::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }
};

}
}


#endif  // REF_COUNTED_SHARED_PTR_LIBCXX_H_
