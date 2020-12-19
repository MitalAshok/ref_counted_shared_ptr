# ref_counted_shared_ptr

A C++11 header-only library that hijacks `std::enable_shared_from_this` so that you can use alternative (non-lifetime based) refcounting.

## Rationale

Say you are interfacing with another language or a library written around a different language, and you need
a class with reference counting where the library needs to be able to affect the reference count. An example:
Window's COM interfaces, [`IUnknown`](https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown)
needing an `AddRef` and `Release`.

Using a `std::shared_ptr`, which would be the obvious implementation for reference counting with `use_count`,
is not feasible since there is no way to request an "increment reference count" and "decrement reference count", since
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

REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS(CInterface);
```

## Usage

Include `ref_counted_shared_ptr/std.h` or `ref_counted_shared_ptr/boost.h` to use
`ref_counted_shared_ptr::std::ref_counted_shared_ptr<T>` or `ref_counted_shared_ptr::boost::ref_counted_shared_ptr<T>`
respectively.

A type `T` must publicly inherit from `ref_counted_shared_ptr::std::ref_counted_shared_ptr<T>` (or
`ref_counted_shared_ptr::boost::ref_counted_shared_ptr<T>` if using `boost::shared_ptr<T>` instead of
`std::shared_ptr<T>`). This provides the member functions `incref`, `decref`, `use_count`, `shared_from_this`,
`weak_from_this`.

Before instantiating any of these functions (by using them),
`REF_COUNTED_SHARED_PTR_DEFINE_PRIVATE_ACCESSORS(T)` must appear somewhere in the global scope (not in any namespace).

Currently, `ref_counted_shared_ptr/std.h` supports:

 * The GNU Standard C++ Library v3 (libstdc++ <https://gcc.gnu.org/onlinedocs/libstdc++/>)
 * "libc++" C++ Standard Library (libc++ <https://libcxx.llvm.org/>)
 * Microsoft's C++ Standard Library (<https://github.com/microsoft/STL>)

## Documentation

```c++
namespace ref_counted_shared_ptr::std {

template<typename Self>
struct ref_counted_shared_ptr : public ::std::enable_shared_from_this<Self> {
protected:
    constexpr ref_counted_shared_ptr() noexcept;
    ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;

    ~ref_counted_shared_ptr();

    ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept;

    long incref() const;

    long decref() const noexcept;

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

template<typename Self>
struct ref_counted_shared_ptr : public ::boost::enable_shared_from_this<Self> {
protected:
    constexpr ref_counted_shared_ptr() noexcept;
    ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept;

    ~ref_counted_shared_ptr();

    ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept;

    long incref() const;

    long decref() const noexcept;

    long use_count() const noexcept;

public:
    ::boost::weak_ptr<Self> weak_from_this() noexcept;
    ::boost::weak_ptr<const Self> weak_from_this() const noexcept;

    // Inherited from ::boost::enable_shared_from_this<Self>
    // ::boost::shared_ptr<Self> shared_from_this();
    // ::boost::shared_ptr<const Self> shared_from_this() const;
};

}
```

### Constructors

```c++
protected:
constexpr ref_counted_shared_ptr() noexcept = default;
ref_counted_shared_ptr(const ref_counted_shared_ptr&) noexcept = default;
```

Only calls base class's (`shared_ptr<Self>`) default or copy constructor.
No move constructor since copying does not transfer reference counts.

### Destructor

```c++
protected:
~ref_counted_shared_ptr() = default;
```

Destroys base class (`shared_ptr<Self>`).

### `operator=`

```c++
protected:
ref_counted_shared_ptr& operator=(const ref_counted_shared_ptr&) noexcept = default;
```

Only calls base class's (`shared_ptr<Self>`) copy-assign operator. This
does not affect any reference counts, and `shared_from_this` and `weak_from_this` are unaffected since
`this` does not change (Only `*this`).

### `incref`

```c++
protected:
long incref() const;
```

Accesses the private `weak_ptr<Self>` member and increments the reference
count. Returns the current reference count after incrementing it (will always be `> 1`).

Must be paired with a call to `decref` afterwards to be able to deallocate `*this`. Before that,
`*this` will be kept alive even if all `shared_ptr<Self>` objects of `this` are destroyed.

Will throw `bad_weak_ptr` if the control block for `this` has not been allocated (which is when
`this->use_count() == 0`). This can happen if `*this` was constructed via `new Self(...)` and
never assigned to a `shared_ptr<Self>`, like:

```c++
T* t = new T;
t->incref();  // Throws `std::bad_weak_ptr`
```

The only other case this can happen is if `incref` is called from within the constructor for `*this`.
This is not allowed for the same reason `shared_from_this` is not allowed to be called in the same context.

When an exceptions is thrown, the reference count is unaffected and nothing happens to `*this`.

This can be avoided by using `make_shared` or `allocate_shared`, or assigning to a local `std::shared_ptr<T>` first:

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
long decref() const noexcept;
```

Accesses the private `weak_ptr<Self>` member and decrements the reference count. Returns the
current reference count after decrementing it (will always be `>= 0`).

Must have called `incref` at least once before, and there can only be one call to `decref` after every call
to `incref`. If there isn't a corresponding call to `incref`, the behaviour is undefined.

If `incref` had been called `n` times before, and this is the `n`th time calling `decref`,
and there are no `shared_ptr<Self>` objects which own `*this`, `*this` is destroyed and `0` is returned.
The converse is also true: If `0` is returned, `*this` has been deleted.

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
Just returns a copy of the private `weak_ptr` member. If this is empty, `use_count()` will return `0`, and
`incref` will throw.

### `shared_from_this`

```c++
public:
// ref_counted_shared_ptr::std::ref_counted_shared_ptr<Self>
// Inherited from ::std::enable_shared_from_this<Self>
// ::std::shared_ptr<Self> shared_from_this();
// ::std::shared_ptr<const Self> shared_from_this() const;

// ref_counted_shared_ptr::boost::ref_counted_shared_ptr<Self>
// Inherited from ::boost::enable_shared_from_this<Self>
// ::boost::shared_ptr<Self> shared_from_this();
// ::boost::shared_ptr<const Self> shared_from_this() const;
```

Equivalent to `shared_ptr<Self>(this->weak_from_this())` or `shared_ptr<const Self>(this->weak_from_this())`.
Will throw `bad_weak_ptr` if `this->weak_from_this()` is empty.
