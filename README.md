ref_counted_shared_ptr
======================

General gist:

```c++
struct test : ref_counted_shared_ptr::std::ref_counted_shared_ptr<test> {
    // Base class defines `shared_from_this`, `weak_from_this`
    // Other stuff
};

REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS(test);

int main() {
    test* p;
    {
        std::shared_ptr<test> s_p = std::make_shared<T>();

        test* p = s_p.get();
        p->incref();
    }
    // p is not destroyed because of the incref
    p->use_count();  // 1. This is #shared_ptrs + #(times incref called)

    {
        std::shared_ptr<test> q = p.shared_from_this();  // Can retrieve pointer
        p->decref();  // Not destroyed because of the shared_pt still exists
    }
    // Now destroyed
}
```

TODO: Documentation
