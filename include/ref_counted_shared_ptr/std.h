#include <memory>


#if defined(_LIBCPP_MEMORY)
#include "ref_counted_shared_ptr/impl/libcxx.h"
#elif defined(_GLIBCXX_MEMORY)
#include "ref_counted_shared_ptr/impl/libstdcxx.h"
#elif defined(_MEMORY_)
#include "ref_counted_shared_ptr/impl/microsoft.h"
#elif !defined(REF_COUNTED_SHARED_PTR_OPTIONAL_STD)
#error "Could not identify your standard library implementation for ref_counted_shared_ptr::std::ref_counted_shared_ptr"
#endif

#if defined(REF_COUNTED_SHARED_PTR_STD) && !defined(REF_COUNTED_SHARED_PTR_STD_DEFINED)
#define REF_COUNTED_SHARED_PTR_STD_DEFINED

#include "ref_counted_shared_ptr/impl/common.h"


namespace ref_counted_shared_ptr {
namespace std {

template<typename Self>
struct typed_ref_counted_shared_ptr : ::std::enable_shared_from_this<Self> {
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<typed_ref_counted_shared_ptr, Self>::value || ::std::is_same<const volatile Self, const volatile void>::value, "std::typed_ref_counted_shared_ptr<Self>: Self must derive from std::typed_ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }

    using implementation = ::ref_counted_shared_ptr::detail::common_implementation<::ref_counted_shared_ptr::detail::std::implementation_information>;

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
    ::std::weak_ptr<Self> weak_from_this() noexcept {
        return static_cast<void>(crtp_checks()), implementation::weak_from_this(*this);
    }
    ::std::weak_ptr<const Self> weak_from_this() const noexcept {
        return static_cast<void>(crtp_checks()), implementation::weak_from_this(*this);
    }
};

struct enable_shared_from_void : ::ref_counted_shared_ptr::std::typed_ref_counted_shared_ptr<void> {};

}
}

REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS_STD(void);

namespace ref_counted_shared_ptr {
namespace std {

template<typename Self = void>
struct ref_counted_shared_ptr : ::ref_counted_shared_ptr::std::enable_shared_from_void {
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value || ::std::is_same<const volatile Self, const volatile void>::value, "std::ref_counted_shared_ptr<Self>: Self must derive from std::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }

    using base = ::ref_counted_shared_ptr::std::enable_shared_from_void;
protected:
    using base::base;
    using base::operator=;
    ~ref_counted_shared_ptr() = default;

    using base::incref;
    using base::decref;
    using base::use_count;
public:
    ::std::shared_ptr<Self> shared_from_this() {
        return static_cast<void>(crtp_checks()), ::std::static_pointer_cast<Self>(::std::shared_ptr<void>(base::weak_from_this()));
    }

    ::std::shared_ptr<const Self> shared_from_this() const {
        return static_cast<void>(crtp_checks()), ::std::static_pointer_cast<const Self>(::std::shared_ptr<const void>(base::weak_from_this()));
    }

    ::std::weak_ptr<Self> weak_from_this() noexcept {
        return static_cast<void>(crtp_checks()), ::std::static_pointer_cast<Self>(base::weak_from_this().lock());
    }

    ::std::weak_ptr<const Self> weak_from_this() const noexcept {
        return static_cast<void>(crtp_checks()), ::std::static_pointer_cast<const Self>(base::weak_from_this().lock());
    }
};

}
}

#endif
