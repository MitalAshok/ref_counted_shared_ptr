#include <iostream>

#define NS std
#include "ref_counted_shared_ptr/std.h"

// #define NS boost
// #include "ref_counted_shared_ptr/boost.h"

struct test : ref_counted_shared_ptr::NS::ref_counted_shared_ptr<test> {
    test() { std::cout << "test::test()\n"; }
    ~test() { std::cout << "test::~test()\n"; }

    // Make public
    using ref_counted_shared_ptr::use_count;
    using ref_counted_shared_ptr::incref;
    using ref_counted_shared_ptr::decref;
};

REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS(test);

int main() {
    test* ptr;
    {
#define STR(X) #X
#define TO_STR(X) STR(X)
        std::cout << TO_STR(NS) "::make_shared<test>()\n";
        auto t = NS::make_shared<test>();
        ptr = t.get();
        std::cout << "t.use_count(): " << t.use_count() << "\nt->use_count(): " << t->use_count() << "\nt->incref()\n";
        t->incref();
        std::cout << "t.use_count(): " << t.use_count() << "\nt->use_count(): " << t->use_count() << "\nshared_ptr<test>::~shared_ptr()\n";
    }
    std::cout << "ptr->use_count(): " << ptr->use_count() << "\nptr->decref()\n";
    ptr->decref();
}
