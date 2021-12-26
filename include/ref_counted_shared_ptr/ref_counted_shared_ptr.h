namespace ref_counted_shared_ptr {}

#include "ref_counted_shared_ptr/impl/redefine_macro.h"

#ifdef BOOST_SMART_PTR_ENABLE_SHARED_FROM_THIS_HPP_INCLUDED
#ifndef REF_COUNTED_SHARED_PTR_BOOST_H_
#include "ref_counted_shared_ptr/boost.h"
#endif
#endif

#ifndef REF_COUNTED_SHARED_PTR_STD
#ifdef REF_COUNTED_SHARED_PTR_OPTIONAL_STD
#include "ref_counted_shared_ptr/std.h"
#else
#define REF_COUNTED_SHARED_PTR_OPTIONAL_STD
#include "ref_counted_shared_ptr/std.h"
#undef REF_COUNTED_SHARED_PTR_OPTIONAL_STD
#endif
#endif
