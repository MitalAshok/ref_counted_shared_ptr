#ifndef REF_COUNTED_SHARED_PTR_ACCESS_PRIVATE_MEMBER_H_
#define REF_COUNTED_SHARED_PTR_ACCESS_PRIVATE_MEMBER_H_

namespace ref_counted_shared_ptr {
namespace detail {

template<typename Tag, typename ClassType, typename DataType>
struct private_member;

template<typename Tag, typename ClassType, typename DataType>
constexpr typename private_member<Tag, ClassType, DataType>::type get_private_member(private_member<Tag, ClassType, DataType>) noexcept;

template<typename Tag, typename ClassType, typename DataType>
struct private_member {
    using class_type = ClassType;
    using data_type = DataType;
    using type = data_type class_type::*;

    constexpr friend type get(private_member) noexcept;

    static constexpr type get_value() noexcept {
        return get(private_member());
    }
};

template<typename Tag, typename Tag::type M>
struct make_private_member {
    friend constexpr typename Tag::type get(private_member<Tag, typename Tag::class_type, typename Tag::data_type>) noexcept {
        return M;
    }
};

}
}

#endif  // REF_COUNTED_SHARED_PTR_ACCESS_PRIVATE_MEMBER_H_
