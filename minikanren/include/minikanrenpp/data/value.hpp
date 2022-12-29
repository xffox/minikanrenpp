#ifndef MINIKANRENPP_DATA_VALUE_HPP
#define MINIKANRENPP_DATA_VALUE_HPP

#include <concepts>
#include <type_traits>
#include <ranges>
#include <vector>
#include <unordered_set>
#include <optional>
#include <iterator>
#include <utility>
#include <ostream>

#include <minikanrenpp/xvariant/variant.hpp>
#include "minikanrenpp/variable.hpp"
#include "minikanrenpp/data/fragmentable.hpp"

namespace minikanrenpp::data
{
    class UnifiableVariant;
    class UnifiableConstRefVariant;

    class ValueStore
    {
    public:
        virtual ~ValueStore() = default;

        virtual bool update(const UnifiableConstRefVariant &left, const UnifiableConstRefVariant &right) = 0;
        virtual bool update(const Variable &left, const UnifiableConstRefVariant &right) = 0;
        virtual bool update(const Variable &left, const Variable &right) = 0;

        virtual std::optional<UnifiableVariant> result(
            std::unordered_set<Variable> &seen,
            const Variable &var) const = 0;
    };

    namespace inner
    {
        template<typename T>
        class TypedUnifiableDescr;

        class UnifiableDescr: public xvariant::Description
        {
        public:
            virtual bool unify(
                ValueStore &store,
                const UnifiableDescr::ConstEphPtr &left,
                const UnifiableConstRefVariant &right) const = 0;
            virtual std::optional<UnifiableVariant> expand(
                std::unordered_set<Variable> &seen,
                const ValueStore &store,
                const UnifiableDescr::ConstEphPtr &value) const = 0;
            virtual std::optional<std::vector<UnifiableConstRefVariant>> tryFragment(
                const UnifiableDescr::ConstEphPtr &value) const = 0;
            // TODO: check if need this to compare types
//            virtual bool sameType();
        };

        template<typename T>
        class TypedUnifiableDescr: public xvariant::TypedDescription<T, UnifiableDescr>
        {
        public:
            bool unify(
                ValueStore &store,
                const UnifiableDescr::ConstEphPtr &left,
                const UnifiableConstRefVariant &right) const override;
            std::optional<UnifiableVariant> expand(
                std::unordered_set<Variable> &seen,
                const ValueStore &store,
                const UnifiableDescr::ConstEphPtr &value) const override;
            std::optional<std::vector<UnifiableConstRefVariant>> tryFragment(
                const UnifiableDescr::ConstEphPtr &value) const override;
        };
    }

    template<template<typename> class TypedDescr,
        template<template<typename> class> class Base>
            requires (std::is_base_of_v<xvariant::SkeletonConstVariant<TypedDescr>,
                Base<TypedDescr>>)
    class SkeletonUnifiableVariant: public Base<TypedDescr>
    {
    public:
        using Base<TypedDescr>::Base;

        bool unify(ValueStore& store, const UnifiableConstRefVariant &that) const;
        std::optional<UnifiableVariant> expand(
            std::unordered_set<Variable> &seen,
            const ValueStore &store) const;
        std::optional<std::vector<UnifiableConstRefVariant>> tryFragment() const;
    };

    template<template<typename> class TypedDescr>
    using BaseConstUnifiableVariant =
        SkeletonUnifiableVariant<TypedDescr, xvariant::SkeletonConstVariant>;

    template<template<typename> class TypedDescr>
    using BaseUnifiableVariant =
        SkeletonUnifiableVariant<TypedDescr, xvariant::SkeletonVariant>;

    class UnifiableVariant: public xvariant::BaseVariant<
                            inner::TypedUnifiableDescr, BaseUnifiableVariant>
    {
    public:
        using xvariant::BaseVariant<
            inner::TypedUnifiableDescr, BaseUnifiableVariant>::BaseVariant;

        template<typename T>
            requires (!std::is_base_of_v<
                xvariant::SkeletonConstVariant<inner::TypedUnifiableDescr>,
                std::decay_t<T>>)
        UnifiableVariant(T &&value)
            :BaseVariant(std::forward<T>(value))
        {}
    };

    class UnifiableConstRefVariant: public xvariant::BaseConstRefVariant<
                                    inner::TypedUnifiableDescr,
                                    BaseConstUnifiableVariant>
    {
    public:
        using xvariant::BaseConstRefVariant<
            inner::TypedUnifiableDescr,
            BaseConstUnifiableVariant>::BaseConstRefVariant;
    };

    class UnifiableRefVariant: public xvariant::BaseRefVariant<
                               inner::TypedUnifiableDescr,
                               BaseUnifiableVariant>
    {
    public:
        using xvariant::BaseRefVariant<
            inner::TypedUnifiableDescr,
            BaseUnifiableVariant>::BaseRefVariant;
    };

    using Value = UnifiableVariant;
    using RefValue = UnifiableRefVariant;
    using ConstRefValue = UnifiableConstRefVariant;
}

namespace minikanrenpp::data
{
    template<template<typename> class TypedDescr,
        template<template<typename> class> class Base>
            requires (std::is_base_of_v<xvariant::SkeletonConstVariant<TypedDescr>,
                Base<TypedDescr>>)
    bool SkeletonUnifiableVariant<TypedDescr, Base>::unify(
        ValueStore& store, const UnifiableConstRefVariant &that) const
    {
        return SkeletonUnifiableVariant::descr().unify(store,
            this->constData(), that);
    }

    template<template<typename> class TypedDescr,
        template<template<typename> class> class Base>
            requires (std::is_base_of_v<xvariant::SkeletonConstVariant<TypedDescr>,
                Base<TypedDescr>>)
    std::optional<UnifiableVariant> SkeletonUnifiableVariant<TypedDescr, Base>::expand(
        std::unordered_set<Variable> &seen,
        const ValueStore &store) const
    {
        return SkeletonUnifiableVariant::descr().expand(seen, store,
            this->constData());
    }

    template<template<typename> class TypedDescr,
        template<template<typename> class> class Base>
            requires (std::is_base_of_v<xvariant::SkeletonConstVariant<TypedDescr>,
                Base<TypedDescr>>)
    std::optional<std::vector<UnifiableConstRefVariant>>
        SkeletonUnifiableVariant<TypedDescr, Base>::tryFragment() const
    {
        return SkeletonUnifiableVariant::descr().tryFragment(this->constData());
    }

    namespace inner
    {
        template<typename T>
        bool TypedUnifiableDescr<T>::unify(
            ValueStore &store,
            const UnifiableDescr::ConstEphPtr &left,
            const UnifiableConstRefVariant &right) const
        {
            const auto &typedLeft = TypedUnifiableDescr::concrete(left);
            if constexpr(std::is_same_v<T, Variable>)
            {
                if(const auto* variableRight = right.valueIf<Variable>())
                {
                    return store.update(typedLeft, *variableRight);
                }
                else
                {
                    return store.update(typedLeft, right);
                }
            }
            else
            {
                if(const auto *variableRight = right.valueIf<Variable>())
                {
                    return store.update(*variableRight,
                        ConstRefValue{typedLeft});
                }
                else
                {
                    if constexpr(Fragmentable<T>)
                    {
                        // TODO: might need to also compare the types
                        if(const auto &maybeRightFragments = right.tryFragment())
                        {
                            auto leftIter = std::ranges::begin(typedLeft);
                            auto rightIter = std::ranges::begin(*maybeRightFragments);
                            for(; leftIter != std::ranges::end(typedLeft) &&
                                rightIter != std::ranges::end(*maybeRightFragments);
                                ++leftIter, ++rightIter)
                            {
                                if(!UnifiableConstRefVariant(*leftIter).unify(
                                        store, *rightIter))
                                {
                                    return false;
                                }
                            }
                            if(leftIter == std::ranges::end(typedLeft) &&
                                rightIter == std::ranges::end(*maybeRightFragments))
                            {
                                return true;
                            }
                        }
                    }
                    return store.update(ConstRefValue{typedLeft}, right);
                }
            }
        }

        template<typename T>
        std::optional<UnifiableVariant> TypedUnifiableDescr<T>::expand(
            std::unordered_set<Variable> &seen,
            const ValueStore &store,
            const UnifiableDescr::ConstEphPtr &value) const
        {
            const auto &typedValue = TypedUnifiableDescr::concrete(value);
            if constexpr(std::is_same_v<T, Variable>)
            {
                return store.result(seen, typedValue);
            }
            else
            {
                if constexpr(Fragmentable<T>)
                {
                    using V = typename T::value_type;
                    std::vector<V> newFragments;
                    for(const auto &fragment : typedValue)
                    {
                        auto maybeNewFragment =
                            UnifiableConstRefVariant(fragment).expand(seen, store);
                        if(!maybeNewFragment)
                        {
                            return std::nullopt;
                        }
                        if constexpr(std::is_same_v<V, Value>)
                        {
                            newFragments.push_back(std::move(*maybeNewFragment));
                        }
                        else
                        {
                            if(!maybeNewFragment->hasType<V>())
                            {
                                return std::nullopt;
                            }
                            newFragments.push_back(
                                std::move(maybeNewFragment->value<V>()));
                        }
                    }
                    return T(newFragments);
                }
                else
                {
                    return UnifiableVariant{typedValue};
                }
            }
        }

        template<typename T>
        std::optional<std::vector<UnifiableConstRefVariant>>
            TypedUnifiableDescr<T>::tryFragment(
                const UnifiableDescr::ConstEphPtr &value) const
        {
            if constexpr(Fragmentable<T>)
            {
                const auto &typedValue = TypedUnifiableDescr::concrete(value);
                return {{std::begin(typedValue), std::end(typedValue)}};
            }
            return std::nullopt;
        }
    }
}

#endif
