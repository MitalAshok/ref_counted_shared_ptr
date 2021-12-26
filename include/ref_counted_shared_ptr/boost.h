#ifndef REF_COUNTED_SHARED_PTR_BOOST_H_
#define REF_COUNTED_SHARED_PTR_BOOST_H_

#include <type_traits>

#include <boost/smart_ptr.hpp>

#include "ref_counted_shared_ptr/impl/boost.h"
#include "ref_counted_shared_ptr/impl/common.h"


namespace ref_counted_shared_ptr {
namespace boost {

template<typename Self>
struct typed_ref_counted_shared_ptr : ::boost::enable_shared_from_this<Self> {
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<typed_ref_counted_shared_ptr, Self>::value || ::std::is_same<const volatile Self, const volatile void>::value, "boost::typed_ref_counted_shared_ptr<Self>: Self must derive from boost::typed_ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }

    using implementation = ::ref_counted_shared_ptr::detail::common_implementation<::ref_counted_shared_ptr::detail::boost::implementation_information>;

protected:
    constexpr typed_ref_counted_shared_ptr() noexcept = default;
    typed_ref_counted_shared_ptr(const typed_ref_counted_shared_ptr&) noexcept = default;

    typed_ref_counted_shared_ptr& operator=(const typed_ref_counted_shared_ptr&) noexcept = default;

    ~typed_ref_counted_shared_ptr() = default;

    long incref() const {
        return static_cast<void>(crtp_checks()), implementation::incref(*this);
    }

    long decref() const {
        return static_cast<void>(crtp_checks()), implementation::decref(*this);
    }

    long use_count() const noexcept {
        return static_cast<void>(crtp_checks()), implementation::use_count(*this);
    }

public:
    ::boost::weak_ptr<Self> weak_from_this() noexcept {
        return static_cast<void>(crtp_checks()), implementation::weak_from_this(*this);
    }
    ::boost::weak_ptr<const Self> weak_from_this() const noexcept {
        return static_cast<void>(crtp_checks()), implementation::weak_from_this(*this);
    }
};

struct enable_shared_from_void : ::ref_counted_shared_ptr::boost::typed_ref_counted_shared_ptr<void> {};

}
}

REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_BOOST(void);

namespace ref_counted_shared_ptr {
namespace boost {

template<typename Self = void>
struct ref_counted_shared_ptr : private ::ref_counted_shared_ptr::boost::enable_shared_from_void {
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value || ::std::is_same<const volatile Self, const volatile void>::value, "boost::ref_counted_shared_ptr<Self>: Self must derive from boost::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }

    using base = ::ref_counted_shared_ptr::boost::enable_shared_from_void;
protected:
    using base::base;
    using base::operator=;
    ~ref_counted_shared_ptr() = default;

    using base::incref;
    using base::decref;
    using base::use_count;
public:
    ::boost::shared_ptr<Self> shared_from_this() noexcept {
        return static_cast<void>(crtp_checks()), ::boost::static_pointer_cast<Self>(::boost::shared_ptr<void>(base::weak_from_this()));
    }

    ::boost::shared_ptr<const Self> shared_from_this() const noexcept {
        return static_cast<void>(crtp_checks()), ::boost::static_pointer_cast<const Self>(::boost::shared_ptr<const void>(base::weak_from_this()));
    }

    ::boost::weak_ptr<Self> weak_from_this() noexcept {
        return this->shared_from_this();
    }

    ::boost::weak_ptr<const Self> weak_from_this() const noexcept {
        return this->shared_from_this();
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_BOOST_H_
