#ifndef REF_COUNTED_SHARED_PTR_STD
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
#endif
