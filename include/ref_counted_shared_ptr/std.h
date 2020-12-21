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
struct ref_counted_shared_ptr : ::std::enable_shared_from_this<Self> {
private:
    static constexpr bool crtp_checks() noexcept {
        static_assert(::std::is_base_of<ref_counted_shared_ptr, Self>::value, "std::ref_counted_shared_ptr<Self>: Self must derive from std::ref_counted_shared_ptr<Self> for CRTP");
        return true;
    }

    using implementation = ::ref_counted_shared_ptr::detail::common_implementation<::ref_counted_shared_ptr::detail::std::implementation_information>;

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
    ::std::weak_ptr<Self> weak_from_this() noexcept {
        return crtp_checks(), implementation::weak_from_this(*this);
    }
    ::std::weak_ptr<const Self> weak_from_this() const noexcept {
        return crtp_checks(), implementation::weak_from_this(*this);
    }
};

}
}

#endif
