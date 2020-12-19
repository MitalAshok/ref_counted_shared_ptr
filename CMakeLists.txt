cmake_minimum_required(VERSION 3.0)
project(ref_counted_shared_ptr)

set(CMAKE_CXX_STANDARD 11)

add_library(ref_counted_shared_ptr INTERFACE)
target_sources(ref_counted_shared_ptr INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/detail/access_private_member.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/impl/boost.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/impl/libcxx.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/impl/libstdcxx.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/impl/microsoft.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/impl/redefine_macro.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/boost.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/std.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ref_counted_shared_ptr/ref_counted_shared_ptr.h
)
target_include_directories(ref_counted_shared_ptr INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include/)

add_executable(ref_counted_shared_ptr_sample ${CMAKE_CURRENT_LIST_DIR}/sample/sample.cpp)
target_link_libraries(ref_counted_shared_ptr_sample PRIVATE ref_counted_shared_ptr)
