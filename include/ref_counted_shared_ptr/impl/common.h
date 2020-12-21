#ifndef REF_COUNTED_SHARED_PTR_COMMON_H_
#define REF_COUNTED_SHARED_PTR_COMMON_H_

#include <cstdlib>


namespace ref_counted_shared_ptr {
namespace detail {

template<typename ImplementationInformation>
struct common_implementation {
    // Required of Details:

    // template<typename T> using shared_ptr = NS::shared_ptr<T>;
    // template<typename T> using weak_ptr = NS::weak_ptr<T>;
    // template<typename T> using enable_shared_from_this = NS::enable_shared_from_this<T>;
    template<typename T>
    using shared_ptr = typename ImplementationInformation::template shared_ptr<T>;
    template<typename T>
    using weak_ptr = typename ImplementationInformation::template weak_ptr<T>;
    template<typename T>
    using enable_shared_from_this = typename ImplementationInformation::template enable_shared_from_this<T>;

    // using control_block_type = /* type of control block that is gotten from a weak_ptr<T> */;
    using control_block_type = typename ImplementationInformation::control_block_type;
    // using atomic_count_type = /* type of reference count held by control block */;
    using atomic_count_type = typename ImplementationInformation::atomic_count_type;
    // using regular_count_type = /* type of value returned by incrementing / decrementing / reading `atomic_count_type` */;
    using regular_count_type = typename ImplementationInformation::regular_count_type;

    // Returns the private weak_ptr<T> member of a enable_shared_from_this<T>, const_cast since it is mutable
    template<typename T>
    static weak_ptr<T>& get_weak_ptr(const enable_shared_from_this<T>& p) noexcept {
        return ImplementationInformation::get_weak_ptr(p);
    }

    // Returns the nullable pointer to a control block held by a weak_ptr<T>
    template<typename T>
    static control_block_type*& get_control_block(weak_ptr<T>& p) noexcept {
        return ImplementationInformation::get_control_block(p);
    }

    // Returns the reference count (number of shared_ptrs) stored on a control block
    static atomic_count_type& get_count(control_block_type& control_block) noexcept {
        return ImplementationInformation::get_count(control_block);
    }

    // Cast to the equivalent `long` value. `static_cast<long>(count)` should suffice, possibly +/- some offset
    static long cast_count_to_long(regular_count_type count) {
        return ImplementationInformation::cast_count_to_long(count);
    }

    // About reference counts:
    // Assume there is some function `regular_count_type fetch(atomic_count_type& count)` which loads the value
    // stored in count.
    // It should be adjusted such that 1 is returned if there is only one pointer,
    // and 0 would be returned after the object has been destroyed when there are no more references.
    // This is exactly the same as what `shared_ptr<T>::use_count` returns.
    //     assert(cast_count_to_long(fetch(get_count(*get_control_block(weak_ptr<T>(shared_ptr<T>(new T)))))) == 1);
    //     weak_ptr<T> expired = shared_ptr<T>(new T); assert(cast_count_to_long(fetch(get_count(*get_countrol_block(expired)))) == 0)

    // Returns the equivalent of `cast_count_to_long(fetch(get_count(control_block)))`. `fetch` can be a relaxed load.
    // Most control block type implementations have a public member function `use_count` used
    // to implement `shared_ptr<T>::use_count`, and that should also suffice.
    static long get_use_count(control_block_type& control_block) noexcept {
        return ImplementationInformation::get_use_count(control_block);
    }

    // Increment count and return it's current value (adjusted the same way as fetch)
    // count will never be 0, so will never return 1.
    static regular_count_type increment_and_fetch(atomic_count_type& count, control_block_type& control_block) noexcept {
        return ImplementationInformation::increment_and_fetch(count, control_block);
    }

    // Decrement count and return it's current value (adjusted the same way as fetch)
    static regular_count_type decrement_and_fetch(atomic_count_type& count, control_block_type& control_block) noexcept {
        return ImplementationInformation::decrement_and_fetch(count, control_block);
    }

    // Called when decrement_and_fetch(get_count(control_block)) returns 0 (and the object should be destroyed)
    // A valid implementation is to call the equivalent of `control_block->add_shared(); control_block->remove_shared()`
    // (No need for atomicity, since this should be called at most once per control block)
    static void on_zero_references(atomic_count_type& count, control_block_type& control_block) noexcept {
        return ImplementationInformation::on_zero_references(count, control_block);
    }

    // Implementation of ref_counted_shared_ptr functions:
    template<typename T>
    static long incref(const enable_shared_from_this<T>& p) {
        control_block_type* control_block = get_control_block(get_weak_ptr(p));

        if (control_block) {
            return cast_count_to_long(increment_and_fetch(get_count(*control_block), *control_block));
        }

        throw_bad_weak_ptr<T>();
    }

    template<typename T>
    static long decref(const enable_shared_from_this<T>& p) {
        control_block_type* control_block = get_control_block(get_weak_ptr(p));
        if (control_block) {
            atomic_count_type& count = get_count(*control_block);
            long new_count = cast_count_to_long(decrement_and_fetch(count, *control_block));
            if (new_count != 0) return new_count;

            on_zero_references(count, *control_block);
            return 0;
        }

        // Immediately called decref() before constructing a shared_ptr or calling incref
        // Or called decref() after &p was already deleted (which is UB anyways)
        throw_bad_weak_ptr<T>();
    }

    template<typename T>
    static long use_count(const enable_shared_from_this<T>& p) noexcept {
        control_block_type* control_block = get_control_block(get_weak_ptr(p));
        if (!control_block) return 0;
        return get_use_count(*control_block);
    }

    template<typename T>
    static weak_ptr<T> weak_from_this(enable_shared_from_this<T>& p) noexcept {
        return get_weak_ptr(p);
    }

    template<typename T>
    static weak_ptr<const T> weak_from_this(const enable_shared_from_this<T>& p) noexcept {
        return get_weak_ptr(p);
    }

    // Helpers
private:
    template<typename T>
    [[noreturn]] static void throw_bad_weak_ptr() {
        static_cast<void>(shared_ptr<const T>(weak_ptr<const T>()));
        ::std::abort();
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_COMMON_H_
