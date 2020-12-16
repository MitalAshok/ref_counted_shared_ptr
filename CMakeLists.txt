cmake_minimum_required(VERSION 3.13.0)
project(ref_counted_shared_ptr)

set(CMAKE_CXX_STANDARD 11)

add_library(ref_counted_shared_ptr INTERFACE)
target_sources(ref_counted_shared_ptr INTERFACE
        include/ref_counted_shared_ptr/detail/access_private_member.h
        include/ref_counted_shared_ptr/impl/boost.h
        include/ref_counted_shared_ptr/impl/libcxx.h
        include/ref_counted_shared_ptr/impl/libstdcxx.h
        include/ref_counted_shared_ptr/impl/microsoft.h
        include/ref_counted_shared_ptr/impl/redefine_macro.h
        include/ref_counted_shared_ptr/boost.h
        include/ref_counted_shared_ptr/std.h
        include/ref_counted_shared_ptr/ref_counted_shared_ptr.h
)
target_include_directories(ref_counted_shared_ptr INTERFACE include/)
