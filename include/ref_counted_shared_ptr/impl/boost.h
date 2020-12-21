#ifndef REF_COUNTED_SHARED_PTR_IMPL_BOOST_H_
#define REF_COUNTED_SHARED_PTR_IMPL_BOOST_H_

#include <type_traits>

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/smart_ptr/detail/sp_counted_base.hpp>

#include "ref_counted_shared_ptr/detail/access_private_member.h"

#define REF_COUNTED_SHARED_PTR_BOOST
#define REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_BOOST(...)           \
template struct ref_counted_shared_ptr::detail::make_private_member<         \
    ::ref_counted_shared_ptr::detail::boost::weak_this_< __VA_ARGS__ >,      \
    &::boost::enable_shared_from_this< __VA_ARGS__ >::weak_this_             \
>;                                                                           \
                                                                             \
template struct ref_counted_shared_ptr::detail::make_private_member<         \
    ::ref_counted_shared_ptr::detail::boost::pn< __VA_ARGS__ >,              \
    &::boost::weak_ptr< __VA_ARGS__ >::pn                                    \
>;                                                                           \
                                                                             \
template<>                                                                   \
struct ref_counted_shared_ptr::detail::boost::defined_private_accessors< __VA_ARGS__ > : ::std::true_type {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"


#ifdef BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_PT_HPP_INCLUDED
namespace ref_counted_shared_ptr { namespace detail { namespace boost {
struct m_ : private_member<m_, ::boost::detail::sp_counted_base, pthread_mutex_t> {};
}}}

namespace ref_counted_shared_ptr { namespace detail {
template struct make_private_member<boost::m_, &::boost::detail::sp_counted_base::m_>;
}}
#endif


namespace ref_counted_shared_ptr {
namespace detail {
namespace boost {

template<typename T>
struct weak_this_ : private_member<weak_this_<T>, ::boost::enable_shared_from_this<T>, ::boost::weak_ptr<T>> {};
template<typename T>
struct pn : private_member<pn<T>, ::boost::weak_ptr<T>, ::boost::detail::weak_count> {};

// Most implementations of the control block use "::boost::detail::atomic_decrement"
// and "::boost::detail::atomic_conditional_increment" to manipulate a member "use_count_".
// Case on the ones that don't, then a general case for the rest.
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
    return result;
}

inline non_atomic_use_count_type atomic_conditional_increment(use_count_type& pw, ::boost::detail::sp_counted_base& ref_counter) noexcept {
    BOOST_VERIFY( pthread_mutex_lock(&ref_counter.*m_::get_value()) == 0 );
    use_count_type r = pw;
    if (r != 0) ++pw;
    BOOST_VERIFY( pthread_mutex_unlock(&ref_counter.*m_::get_value()) == 0 );
    return r;
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

template<typename T>
struct defined_private_accessors : ::std::false_type {};

}
}
}

namespace ref_counted_shared_ptr {
namespace detail {

template struct make_private_member<boost::pi_, &::boost::detail::weak_count::pi_>;
template struct make_private_member<boost::use_count_, &::boost::detail::sp_counted_base::use_count_>;

}
}

namespace ref_counted_shared_ptr {
namespace detail {
namespace boost {

struct implementation_information {
    template<typename T> using shared_ptr = ::boost::shared_ptr<T>;
    template<typename T> using weak_ptr = ::boost::weak_ptr<T>;
    template<typename T> using enable_shared_from_this = ::boost::enable_shared_from_this<T>;

    using control_block_type = ::boost::detail::sp_counted_base;
    using atomic_count_type = ::ref_counted_shared_ptr::detail::boost::use_count_type;
    using regular_count_type = ::ref_counted_shared_ptr::detail::boost::non_atomic_use_count_type;

    template<typename T>
    static weak_ptr<T>& get_weak_ptr(const enable_shared_from_this<T>& p) noexcept {
        static_assert(
            ::ref_counted_shared_ptr::detail::boost::defined_private_accessors<T>::value,
            "boost::ref_counted_shared_ptr<Self>: Must have REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_BOOST(Self) in the :: namespace scope at some point before attempting to use incref(), decref() or use_count()"
        );

        return const_cast<::boost::weak_ptr<T>&>(p.*::ref_counted_shared_ptr::detail::boost::weak_this_<T>::get_value());
    }

    template<typename T>
    static control_block_type*& get_control_block(weak_ptr<T>& p) noexcept {
        return p.*pn<T>::get_value().*::ref_counted_shared_ptr::detail::boost::pi_::get_value();
    }

    static atomic_count_type& get_count(control_block_type& control_block) noexcept {
        return control_block.*::ref_counted_shared_ptr::detail::boost::use_count_::get_value();
    }

    static long cast_count_to_long(regular_count_type count) {
        return static_cast<long>(count);
    }

    static long get_use_count(control_block_type& control_block) noexcept {
        return control_block.use_count();
    }

    static regular_count_type increment_and_fetch(atomic_count_type& count, control_block_type& control_block) noexcept {
        return ::ref_counted_shared_ptr::detail::boost::atomic_conditional_increment(count, control_block) + 1;
    }

    static regular_count_type decrement_and_fetch(atomic_count_type& count, control_block_type& control_block) noexcept {
        return ::ref_counted_shared_ptr::detail::boost::atomic_decrement(count, control_block) - 1;
    }

    static void on_zero_references(atomic_count_type&, control_block_type& control_block) noexcept {
        control_block.add_ref_copy();
        control_block.release();
    }
};

}
}
}

#endif  // REF_COUNTED_SHARED_PTR_IMPL_BOOST_H_
