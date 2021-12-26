# ref_counted_shared_ptr

A C++11 header-only library that hijacks `std::enable_shared_from_this` so that you can use alternative (non-lifetime based) refcounting.

## Rationale

Say you are interfacing with another language or a library written around a different language, and you need
a class with reference counting where the library needs to be able to use the reference count. An example:
Window's COM interfaces, [`IUnknown`](https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown)
needing an `AddRef` and `Release`.

Using a `std::shared_ptr`, which would be the obvious implementation for reference counting with `use_count`,
is not feasible since there is no way to request an "increment reference count" or "decrement reference count", since
those are tied to RAII: increment on construction, decrement on destruction.

This explicitly adds `incref` and `decref` member functions. With the COM interface example:

```c++
struct CInterfaceName : IInterfaceName, ref_counted_shared_ptr::std::ref_counted_shared_ptr<CInterfaceName> {
    ULONG STDMETHODCALLTYPE AddRef() override {
        return incref();
    }
    ULONG STDMETHODCALLTYPE Release() override {
        return decref();
    }
};
```

## Usage

Include `ref_counted_shared_ptr/std.h` or `ref_counted_shared_ptr/boost.h` to use
`ref_counted_shared_ptr::std::ref_counted_shared_ptr<T>` or `ref_counted_shared_ptr::boost::ref_counted_shared_ptr<T>`
respectively.

A type `T` must publicly inherit from `ref_counted_shared_ptr::std::ref_counted_shared_ptr<T>` (or
`ref_counted_shared_ptr::boost::ref_counted_shared_ptr<T>` if using `boost::shared_ptr<T>` instead of
`std::shared_ptr<T>`). This provides the member functions `incref`, `decref`, `use_count`, `shared_from_this`,
`weak_from_this`.

Due to a quirk of how this is implemented with private member accessors, `ref_counted_shared_ptr` inherits from
`std::enable_shared_from_this<void>`. To truly inherit from `std::enable_shared_from_this<T>` if needed (and for a
possible slight performance boost), `typed_ref_counted_shared_ptr` can be used instead. However, before instantiating any
of the member functions of `typed_ref_counted_shared_ptr<T>` (by using them),
`REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS(T)` must appear somewhere at file scope in the global (`::`) namespace.

Currently, `ref_counted_shared_ptr/std.h` supports:

 * The GNU Standard C++ Library v3 (libstdc++ <https://gcc.gnu.org/onlinedocs/libstdc++/>)
 * "libc++" C++ Standard Library (libc++ <https://libcxx.llvm.org/>)
 * Microsoft's C++ Standard Library (<https://github.com/microsoft/STL>)

## Documentation

```c++
namespace ref_counted_shared_ptr::std {

template<typename Self = void>
struct ref_counted_shared_ptr : public enable_shared_from_void {
protected:
    ~ref_counted_shared_ptr() = default;

    // Inherited from enable_shared_from_void:
    // constexpr ref_counted_shared_ptr() noexcept;
    // ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;

    // ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept;

    // long incref() const;
    // long decref() const;
    // long use_count() const noexcept;
public:
    ::std::weak_ptr<Self> weak_from_this() noexcept;
    ::std::weak_ptr<const Self> weak_from_this() const noexcept;

    ::std::shared_ptr<Self> shared_from_this();
    ::std::shared_ptr<const Self> shared_from_this() const;
};

struct enable_shared_from_void : typed_ref_counted_shared_ptr<void> {
    // Inherits all methods from typed_ref_counted_shared_ptr<void>
};

template<typename Self>
struct typed_ref_counted_shared_ptr {
protected:
    ~typed_ref_counted_shared_ptr() = default;

    constexpr typed_ref_counted_shared_ptr() noexcept;
    typed_ref_counted_shared_ptr(const typed_ref_counted_shared_ptr&) noexcept = default;

    typed_ref_counted_shared_ptr& operator=(const typed_ref_counted_shared_ptr&) noexcept;

    long incref() const;
    long decref() const;
    long use_count() const noexcept;
public:
    ::std::weak_ptr<Self> weak_from_this() noexcept;
    ::std::weak_ptr<const Self> weak_from_this() const noexcept;

    // Inherited from ::std::enable_shared_from_this<Self>
    // ::std::shared_ptr<Self> shared_from_this();
    // ::std::shared_ptr<const Self> shared_from_this() const;
};

}

// boost version is the same replacing `std::shared_ptr` and similar with `boost::shared_ptr` and similar.
namespace ref_counted_shared_ptr::boost {

template<typename Self = void>
struct ref_counted_shared_ptr : public enable_shared_from_void {
    // See std version
};

struct enable_shared_from_void : typed_ref_counted_shared_ptr<void> {
    // Inherits all methods from typed_ref_counted_shared_ptr<void>
};

template<typename Self>
struct typed_ref_counted_shared_ptr {
    // See std version
};

}
```

All functions on all of these classes and class templates do the same thing, except that
`typed_ref_counted_shared_ptr<T>` requires `REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS(T)`
as stated above.

In the following member function documentation, the name of the class will be
taken as `ref_counted_shared_ptr<Self>`, but it equally applies to all three entities. `T` will be
`void` or `Self` depending on if `typed_ref_counted_shared_ptr` is being used.

### Constructors

```c++
protected:
constexpr ref_counted_shared_ptr() noexcept = default;
ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;
```

Only calls base class's (`enable_shared_from_this<T>`) default or copy constructor.
No move constructor since copying does not transfer reference counts, rather an entirely
new reference count.

### Destructor

```c++
protected:
~ref_counted_shared_ptr() = default;
```

Destroys base class (`enable_shared_from_this<T>`).

### `operator=`

```c++
protected:
ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept = default;
```

Only calls base class's (`enable_shared_from_this<Self>`) copy-assign operator. This
does not affect any reference counts, and `shared_from_this` and `weak_from_this` are unaffected since
`this` does not change (Only `*this`).

### `incref`

```c++
protected:
long incref() const;
```

Accesses the private `weak_ptr<T>` member and increments the reference
count. Returns the current reference count after incrementing it (will always be `> 1`).

Must be paired with a call to `decref` afterwards to be able to deallocate `*this`. Before that,
`*this` will be kept alive even if all `shared_ptr<T>` objects of `this` are destroyed.

Will throw `bad_weak_ptr` if the control block for `this` has not been allocated (which is when
`this->use_count() == 0`). This means that `incref` cannot be called inside the constructor of `Self`.
It also means that `this` has to be assigned to a `shared_ptr` at least once before, and this will throw:

```c++
Self* t = new Self;
t->incref();  // Throws `std::bad_weak_ptr`
```

When an exceptions is thrown, the reference count is unaffected and nothing happens to `*this`.

This can be avoided by using `make_shared` or `allocate_shared`, or assigning to a local `std::shared_ptr<Self>` first:

```c++
T* t;
{
    std::shared_ptr<T> s_t = new T;  // or std::make_shared<T>() if possible
    t = s_t.get();
    t->incref();  // incref before the std::shared_ptr is destroyed
}
```

### `decref`

```c++
protected:
long decref() const;
```

Accesses the private `weak_ptr<Self>` member and decrements the reference count. Returns the
current reference count after decrementing it (will always be `>= 0`).

Must have called `incref` at least once before, and there can only be one call to `decref` after every call
to `incref`. If there isn't a corresponding call to `incref`, the behaviour is undefined.

If `incref` had been called `n` times before, and this is the `n`th time calling `decref`,
and there are no `shared_ptr<Self>` objects which own `*this`, `*this` is destroyed and `0` is returned.
The converse is also true: If `0` is returned, `*this` has been destroyed.

### `use_count`

```c++
protected:
long use_count() const noexcept;
```

Returns an approximate value for the current reference count + the number of `shared_ptr<Self>` objects which
refer to `*this`. Will always be `>= 0`.

If this returns `0`, the value is also exact and `incref()` will throw (`this` has never been assigned to a
`shared_ptr<Self>`, `this->weak_from_this()` will be empty).

In a non multi-threaded environment, this is always exact. It is only inexact when not synchronised with respect
to `incref`/`decref`/`shared_ptr<Self>::shared_ptr`/`shared_ptr<Self>::operator=` calls on other thread, which
can affect the reference count.

Equivalent to `this->weak_from_this().use_count()`. Similar to `this->shared_from_this().use_count() - 1` and
`(this->incref(), this->decref())` (other than a `bad_weak_ref` exception).

### `weak_from_this`

```c++
public:
// ref_counted_shared_ptr::std::ref_counted_shared_ptr<Self>
::std::weak_ptr<Self> weak_from_this() noexcept;
::std::weak_ptr<const Self> weak_from_this() const noexcept;

// ref_counted_shared_ptr::boost::ref_counted_shared_ptr<Self>
::boost::weak_ptr<Self> weak_from_this() noexcept;
::boost::weak_ptr<const Self> weak_from_this() const noexcept;
```

Equivalent to C++17's `std::enable_shared_from_this<Self>::weak_from_this`, but also available in C++11.
Just returns a copy of the private `weak_ptr` member (casting it if `T` is `void`).
If this is empty, `use_count()` will return `0`, and `incref` will throw.

### `shared_from_this`

```c++
public:
// Possibly Inherited from ::std::enable_shared_from_this<Self>
::std::shared_ptr<Self> shared_from_this();
::std::shared_ptr<const Self> shared_from_this() const;
```

Equivalent to `shared_ptr<Self>(this->weak_from_this())` or `shared_ptr<const Self>(this->weak_from_this())`.
Will throw `bad_weak_ptr` if `this->weak_from_this()` is empty. If this is `typed_ref_counted_shared_ptr`, this
is inherited from `::std::enable_shared_from_this<Self>`. Otherwise, equivalent to
`static_pointer_cast<c T>(std::enable_shared_from_this<void>::shared_from_this())`, where `c` may possibly be `const`.
