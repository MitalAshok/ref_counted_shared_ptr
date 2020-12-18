#ifndef REF_COUNTED_SHARED_PTR_IMPL_BOOST_H_
#define REF_COUNTED_SHARED_PTR_IMPL_BOOST_H_

#include <type_traits>

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/smart_ptr/detail/sp_counted_base.hpp>

#include "ref_counted_shared_ptr/detail/access_private_member.h"

#define REF_COUNTED_SHARED_PTR_BOOST
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_BOOST(...)           \
template struct ::ref_counted_shared_ptr::detail::make_private_member<       \
    ::ref_counted_shared_ptr::detail::boost::weak_this_< __VA_ARGS__ >,      \
    &::boost::enable_shared_from_this< __VA_ARGS__ >::weak_this_             \
>;                                                                           \
                                                                             \
template struct ::ref_counted_shared_ptr::detail::make_private_member<       \
    ::ref_counted_shared_ptr::detail::boost::pn< __VA_ARGS__ >,              \
    &::boost::weak_ptr< __VA_ARGS__ >::pn                                    \
>;                                                                           \
                                                                             \
template<>                                                                   \
struct ::ref_counted_shared_ptr::detail::boost::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"


#ifdef BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_PT_HPP_INCLUDED
namespace ref_counted_shared_ptr {
namespace detail {
namespace boost {
struct m_ : private_member<m_, ::boost::detail::sp_counted_base, pthread_mutex_t> {};
}

template struct make_private_member<boost::m_, &::boost::detail::sp_counted_base::m_>;
}
}
#endif


namespace ref_counted_shared_ptr {
namespace detail {
namespace boost {  // v ref_counted_shared_ptr::detail::boost

template<typename T>
struct weak_this_ : private_member<weak_this_<T>, ::boost::enable_shared_from_this<T>, ::boost::weak_ptr<T>> {};
template<typename T>
struct pn : private_member<pn<T>, ::boost::weak_ptr<T>, ::boost::detail::weak_count> {};

#if defined(BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_NT_HPP_INCLUDED)
using use_count_type = ::boost::int_least32_t;
using non_atomic_use_count_type = use_count_type;

inline non_atomic_use_count_type atomic_decrement(use_count_type& pw, ::boost::detail::sp_counted_base&) noexcept {
    return pw--;
}

inline non_atomic_use_count_type atomic_conditional_increment(use_count_type& pw, ::boost::detail::sp_counted_base&) noexcept {
    long r = pw;
    if (r != 0) ++pw;
    return r;
}
#elif defined(BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_PT_HPP_INCLUDED)
using use_count_type = ::boost::int_least32_t;
using non_atomic_use_count_type = use_count_type;

inline non_atomic_use_count_type atomic_decrement(use_count_type& pw, ::boost::detail::sp_counted_base& ref_counter) neoxcept {
    BOOST_VERIFY( pthread_mutex_lock(&ref_counter.*m_::get_value()) == 0 );
    use_count_type result = pw--;
    BOOST_VERIFY( pthread_mutex_unlock(&ref_counter.*m_::get_value()) == 0 );
}

inline non_atomic_use_count_type atomic_conditional_increment(use_count_type& pw, ::boost::detail::sp_counted_base& ref_counter) noexcept {
    BOOST_VERIFY( pthread_mutex_lock(&ref_counter.*m_::get_value()) == 0 );
    long r = pw;
    if (r != 0) ++pw;
    return r;
    BOOST_VERIFY( pthread_mutex_unlock(&ref_counter.*m_::get_value()) == 0 );
}
#elif defined(BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_W32_HPP_INCLUDED)
using use_count_type = long;
using non_atomic_use_count_type = use_count_type;

inline non_atomic_use_count_type atomic_decrement(use_count_type& pw, ::boost::detail::sp_counted_base&) noexcept {
    return BOOST_SP_INTERLOCKED_DECREMENT(pw) + 1;
}

inline non_atomic_use_count_type atomic_conditional_increment(use_count_type& pw, ::boost::detail::sp_counted_base&) noexcept {
    for (;;) {
        long tmp = static_cast<long const volatile&>(pw);
        if (tmp == 0) return 0;

#if defined( BOOST_MSVC ) && BOOST_WORKAROUND( BOOST_MSVC, == 1200 )
        // work around a code generation bug
        long tmp2 = tmp + 1;
        if( BOOST_SP_INTERLOCKED_COMPARE_EXCHANGE( &use_count_, tmp2, tmp ) == tmp2 - 1 ) return tmp2 - 1;
#else
        if( BOOST_SP_INTERLOCKED_COMPARE_EXCHANGE( &use_count_, tmp + 1, tmp ) == tmp ) return tmp;
#endif
    }
}
#else
template<typename T1, typename T2>
struct type_pair {
    using first = T1;
    using second = T2;
};

template<typename Ret, typename Arg>
type_pair<Ret, Arg> deduce_types(Ret(*)(Arg*)) { return {}; }

using use_count_type = decltype(deduce_types(&::boost::detail::atomic_decrement))::second;
using non_atomic_use_count_type = decltype(deduce_types(&::boost::detail::atomic_decrement))::first;

inline non_atomic_use_count_type atomic_decrement(use_count_type& pw, ::boost::detail::sp_counted_base&) noexcept {
    return ::boost::detail::atomic_decrement(&pw);
}

inline non_atomic_use_count_type atomic_conditional_increment(use_count_type& pw, ::boost::detail::sp_counted_base&) noexcept {
    return ::boost::detail::atomic_conditional_increment(&pw);
}
#endif

struct pi_ : private_member<pi_, ::boost::detail::weak_count, ::boost::detail::sp_counted_base*> {};
struct use_count_ : private_member<use_count_, ::boost::detail::sp_counted_base, use_count_type> {};
}  // ^ ref_counted_shared_ptr::detail::boost, v ref_counted_shared_ptr::detail
template struct make_private_member<boost::pi_, &::boost::detail::weak_count::pi_>;
template struct make_private_member<boost::use_count_, &::boost::detail::sp_counted_base::use_count_>;
namespace boost {  // ^ ref_counted_shared_ptr::detail, v ref_counted_shared_ptr::detail::boost

template<typename T>
struct defined_private_accessors : ::std::false_type {};

template<typename T>
::boost::weak_ptr<T>& get_weak_ptr(const ::boost::enable_shared_from_this<T>& p) noexcept {
    static_assert(defined_private_accessors<T>::value, "boost::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_BOOST(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()");
    // const_cast is fine since weak_this_ is mutable
    return const_cast<::boost::weak_ptr<T>&>(p.*weak_this_<T>::get_value());
}

template<typename T>
::boost::detail::sp_counted_base*& get_control_block(const ::boost::enable_shared_from_this<T>& p) noexcept {
    return get_weak_ptr(p).*pn<T>::get_value().*pi_::get_value();
}

inline use_count_type& get_count(::boost::detail::sp_counted_base& control_block) noexcept {
    return control_block.*use_count_::get_value();
}

}
}
}  // ^ ref_counted_shared_ptr::detail::boost, v ::

namespace ref_counted_shared_ptr {
namespace boost {

template<typename Self>
struct ref_counted_shared_ptr : ::boost::enable_shared_from_this<Self> {
protected:
    constexpr ref_counted_shared_ptr() noexcept = default;
    ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;

    ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept = default;

    ~ref_counted_shared_ptr() = default;

    long incref() const {
        crtp_checks();

        ::boost::detail::sp_counted_base*& control_block = ::ref_counted_shared_ptr::detail::boost::get_control_block(*this);
        if (control_block) {
            return static_cast<long>(::ref_counted_shared_ptr::detail::boost::atomic_conditional_increment(::ref_counted_shared_ptr::detail::boost::get_count(*control_block), *control_block)) + 1;
        }

        // First time. Create control block
        ::boost::shared_ptr<const Self> s = this->shared_from_this();  // May throw
        return static_cast<long>(::ref_counted_shared_ptr::detail::boost::atomic_conditional_increment(::ref_counted_shared_ptr::detail::boost::get_count(*control_block), *control_block));
    }

    long decref() const noexcept {
        crtp_checks();

        // Must have control block to call decref
        ::boost::detail::sp_counted_base& control_block = *::ref_counted_shared_ptr::detail::boost::get_control_block(*this);
        ::ref_counted_shared_ptr::detail::boost::use_count_type& count = ::ref_counted_shared_ptr::detail::boost::get_count(control_block);
        ::ref_counted_shared_ptr::detail::boost::non_atomic_use_count_type old_count = ::ref_counted_shared_ptr::detail::boost::atomic_decrement(count, control_block);

        if (old_count == 1) {
            // *this must be the only reference, so no race conditions
            control_block.add_ref_copy();
            control_block.release();
            return 0;
        }
        return static_cast<long>(old_count - 1);
    }

    long use_count() const noexcept {
        crtp_checks();

        ::boost::detail::sp_counted_base* control_block = ::ref_counted_shared_ptr::detail::boost::get_control_block(*this);
        if (!control_block) return 0;
        return control_block->use_count();
    }

    ::boost::weak_ptr<Self> weak_from_this() noexcept {
        return ::ref_counted_shared_ptr::detail::boost::get_weak_ptr(*this);
    }
    ::boost::weak_ptr<const Self> weak_from_this() const noexcept {
        return ::ref_counted_shared_ptr::detail::boost::get_weak_ptr(*this);
    }
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value, "boost::ref_counted_shared_ptr<Self>: Self must derive from boost::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_IMPL_BOOST_H_
