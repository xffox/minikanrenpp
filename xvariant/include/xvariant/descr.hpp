#ifndef XVARIANT_TYPEDESCR_HPP
#define XVARIANT_TYPEDESCR_HPP

#include <concepts>
#include <type_traits>
#include <utility>
#include <cassert>

namespace xvariant
{
    class Description
    {
    public:
        using EphPtr = void*;
        using ConstEphPtr = const void*;

    public:
        virtual ~Description() = default;

        virtual void destruct(EphPtr &data) const = 0;
        virtual EphPtr clone(const ConstEphPtr &data) const = 0;
        virtual bool compare(const ConstEphPtr &left, const ConstEphPtr &right) const = 0;
    };

    template<typename T, typename Base_ = Description>
        requires (std::is_base_of_v<Description, Base_>)
    class TypedDescription: public Base_
    {
    public:
        using Base = Base_;

    public:
        template<typename... Args>
        typename TypedDescription::EphPtr construct(Args &&...args) const
        {
            return typename TypedDescription::EphPtr(
                new T{std::forward<Args>(args)...});
        }
        void destruct(typename TypedDescription::EphPtr &data) const override final
        {
            delete &concrete(data);
        }
        typename TypedDescription::EphPtr clone(
            const typename TypedDescription::ConstEphPtr &data) const override final
        {
            assert(data);
            return typename TypedDescription::EphPtr(new T(concrete(data)));
        }
        bool compare(const typename TypedDescription::ConstEphPtr &left,
            const typename TypedDescription::ConstEphPtr &right) const override final
        {
            assert(left);
            assert(right);
            return concrete(left) == concrete(right);
        }

        template<typename P>
            requires (std::same_as<P, typename TypedDescription::EphPtr> ||
                std::same_as<P, typename TypedDescription::ConstEphPtr>)
        static decltype(auto) concrete(const P &ptr)
        {
            assert(ptr);
            using V = std::remove_pointer_t<P>;
            using R = std::conditional_t<std::is_const_v<V>, std::add_const_t<T>, T>;
            return *static_cast<R*>(ptr);
        }
    };
}

constexpr bool operator==(const xvariant::Description &left,
    const xvariant::Description &right)
{
    return &left == &right;
}

#endif
