#ifndef REF_COUNTED_SHARED_PTR_ACCESS_PRIVATE_MEMBER_H_
#define REF_COUNTED_SHARED_PTR_ACCESS_PRIVATE_MEMBER_H_

namespace ref_counted_shared_ptr {
namespace detail {

template<typename Tag, typename ClassType, typename DataType>
struct private_member;

template<typename Tag, typename ClassType, typename DataType>
struct private_member {
    using class_type = ClassType;
    using data_type = DataType;
    using type = data_type class_type::*;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
// In a very early GCC version (GCC 3?), the behaviour of friend function declarations in template classes
// would act as if they were friend functions of templates, even without specifying as `get<private_member>`,
// so this warning warns on potentially broken code. But that is not the case here, since we have multiple
// non-template `get(private_member<Tag, ClassType, DataType>)` functions, which we use `friend` to declare
// the existence of.
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif
    constexpr friend typename private_member::type get(private_member) noexcept;
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    static constexpr type get_value() noexcept {
        return get(private_member());
    }
};

template<typename Tag, typename Tag::type M>
struct make_private_member {
    using pm_t = private_member<Tag, typename Tag::class_type, typename Tag::data_type>;

    friend constexpr typename pm_t::type get(pm_t) noexcept {
        return M;
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_ACCESS_PRIVATE_MEMBER_H_
