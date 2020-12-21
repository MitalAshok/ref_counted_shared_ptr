#ifndef REF_COUNTED_SHARED_PTR_BOOST_H_
#define REF_COUNTED_SHARED_PTR_BOOST_H_

#include <type_traits>

#include <boost/smart_ptr.hpp>

#include "ref_counted_shared_ptr/impl/boost.h"
#include "ref_counted_shared_ptr/impl/common.h"


namespace ref_counted_shared_ptr {
namespace boost {

template<typename Self>
struct ref_counted_shared_ptr : ::boost::enable_shared_from_this<Self> {
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value, "boost::ref_counted_shared_ptr<Self>: Self must derive from boost::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }

    using implementation = ::ref_counted_shared_ptr::detail::common_implementation<::ref_counted_shared_ptr::detail::boost::implementation_information>;

protected:
    constexpr ref_counted_shared_ptr() noexcept = default;
    ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;

    ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept = default;

    ~ref_counted_shared_ptr() = default;

    long incref() const {
        return crtp_checks(), implementation::incref(*this);
    }

    long decref() const {
        return crtp_checks(), implementation::decref(*this);
    }

    long use_count() const noexcept {
        return crtp_checks(), implementation::use_count(*this);
    }

public:
    ::boost::weak_ptr<Self> weak_from_this() noexcept {
        return crtp_checks(), implementation::weak_from_this(*this);
    }
    ::boost::weak_ptr<const Self> weak_from_this() const noexcept {
        return crtp_checks(), implementation::weak_from_this(*this);
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_BOOST_H_
