#ifndef MINIKANRENPP_XVARIANT_VARIANT_H
#define MINIKANRENPP_XVARIANT_VARIANT_H

#include <concepts>
#include <type_traits>
#include <utility>
#include <memory>
#include <cassert>
#include <stdexcept>

#include "minikanrenpp/xvariant/descr.hpp"

namespace minikanrenpp::xvariant
{
    template<typename T>
    using BaseTypedDescr = TypedDescription<T, Description>;

    template<typename T>
    struct TypeTag{};

    template<template<typename> class TypedDescr>
    class SkeletonConstVariant
    {
    public:
        using Descr = typename TypedDescr<int>::Base;

    public:
        SkeletonConstVariant(const Descr *descr)
            :descr_(descr)
        {}
        virtual ~SkeletonConstVariant() = default;

        bool isValid() const
        {
            return static_cast<bool>(descr_);
        }

        template<typename T>
        bool hasType() const
        {
            return descr_ && typeDescr<T>() == *descr_;
        }

        friend bool operator==(const SkeletonConstVariant<TypedDescr> &left,
            const SkeletonConstVariant<TypedDescr> &right)
        {
            if(equalType(left, right))
            {
                assert(left.descr_);
                return left.descr_->compare(
                    left.constData(), right.constData());
            }
            return false;
        }

        friend bool equalType(const SkeletonConstVariant<TypedDescr> &left,
            const SkeletonConstVariant<TypedDescr> &right)
        {
            return left.descr_ && right.descr_ && *left.descr_ == *right.descr_;
        }

    protected:
        template<typename T>
        using CurTypedDescr = TypedDescr<std::decay_t<T>>;

    protected:
        virtual typename Descr::ConstEphPtr constData() const = 0;

        const Descr &descr() const
        {
            assert(descr_);
            return *descr_;
        }

        template<typename T>
        static const CurTypedDescr<T> &typeDescr()
        {
            return prepareTypeDescr<std::decay_t<T>>();
        }

    protected:
        const Descr *descr_;

    private:
        template<typename T>
        static const CurTypedDescr<T> &prepareTypeDescr()
        {
            static CurTypedDescr<T> inst;
            return inst;
        }
    };

    template<template<typename> class TypedDescr>
    class SkeletonVariant: public SkeletonConstVariant<TypedDescr>
    {
    public:
        using SkeletonConstVariant<TypedDescr>::SkeletonConstVariant;

    protected:
        virtual typename SkeletonVariant::Descr::EphPtr data() const = 0;
    };

    template<template<typename> class TypedDescr,
        template<template<typename> class> class Base>
            requires (std::is_base_of_v<SkeletonConstVariant<TypedDescr>,
                Base<TypedDescr>>)
    class BaseConstRefVariant;

    template<template<typename> class TypedDescr = BaseTypedDescr,
        template<template<typename> class> class Base = SkeletonVariant>
            requires (std::is_base_of_v<SkeletonVariant<TypedDescr>,
                Base<TypedDescr>>)
    class BaseVariant: public Base<TypedDescr>
    {
        template<template<typename> class T,
            template<template<typename> class> class B>
                requires (std::is_base_of_v<SkeletonConstVariant<T>, B<T>>)
        friend class BaseConstRefVariant;
        template<template<typename> class T,
            template<template<typename> class> class B>
                requires (std::is_base_of_v<SkeletonVariant<T>,
                    B<T>>)
        friend class BaseRefVariant;

    public:
        template<typename T>
            requires std::default_initializable<T>
        BaseVariant(TypeTag<T>)
            :BaseVariant(T{})
        {}

        template<typename T>
            requires (!std::is_base_of_v<SkeletonConstVariant<TypedDescr>, std::decay_t<T>>)
        explicit BaseVariant(T &&value)
            :Base<TypedDescr>(&BaseVariant::template typeDescr<T>()),
            data_(BaseVariant::template typeDescr<T>().construct(
                    std::forward<T>(value)),
                EphDeleter(BaseVariant::descr_))
        {}

        template<template<typename> class T,
            template<template<typename> class> class B>
        BaseVariant(const BaseConstRefVariant<T, B> &that)
            :BaseVariant(that.descr(), that.constData())
        {}

        BaseVariant(const BaseVariant &that)
            :BaseVariant(that.descr(), that.constData())
        {}

        BaseVariant(BaseVariant &&that)
            :Base<TypedDescr>(that.descr_),
            data_(std::move(that.data_))
        {
            that.descr_ = nullptr;
            that.data_.reset();
        }

        BaseVariant &operator=(const BaseVariant &that)
        {
            if(this == &that)
            {
                return *this;
            }
            if(that.descr_)
            {
                EphUniquePtr newData(
                    that.descr_->clone(that.constData()),
                    EphDeleter(that.descr_));
                data_ = std::move(newData);
                BaseVariant::descr_ = that.descr_;
            }
            else
            {
                data_.reset();
                BaseVariant::descr_ = nullptr;
            }
            return *this;
        }

        BaseVariant &operator=(BaseVariant &&that)
        {
            if(this == &that)
            {
                return *this;
            }
            data_ = std::move(that.data_);
            BaseVariant::descr_ = that.descr_;
            that.descr_ = nullptr;
            that.data_.reset();
            return *this;
        }

        template<typename T>
        std::remove_const_t<T> *valueIf()
        {
            return valueIf<T>(*this);
        }

        template<typename T>
        const T *valueIf() const
        {
            return valueIf<T>(*this);
        }

        template<typename T>
        std::remove_const_t<T> &value()
        {
            return value<T>(*this);
        }

        template<typename T>
        const T &value() const
        {
            return value<T>(*this);
        }

    protected:
        BaseVariant(const typename BaseVariant::Descr &descr,
            const typename BaseVariant::Descr::ConstEphPtr &data)
            :Base<TypedDescr>(&descr),
            data_(BaseVariant::descr_
                ?EphUniquePtr(BaseVariant::descr_->clone(data),
                    EphDeleter(BaseVariant::descr_))
                :EphUniquePtr(nullptr, EphDeleter(BaseVariant::descr_)))
        {}

        typename BaseVariant::Descr::ConstEphPtr constData() const override
        {
            assert(data_);
            return data_.get();
        }

        typename BaseVariant::Descr::EphPtr data() const override
        {
            assert(data_);
            return data_.get();
        }

    private:
        class EphDeleter
        {
        public:
            EphDeleter(const typename BaseVariant::Descr *descr)
                :descr(descr)
            {}
            void operator()(typename BaseVariant::Descr::EphPtr data) const
            {
                descr->destruct(data);
            }
        private:
            const typename BaseVariant::Descr *descr;
        };
        using EphUniquePtr = std::unique_ptr<std::remove_pointer_t<
            typename BaseVariant::Descr::EphPtr>, EphDeleter>;

    private:
        template<typename T, typename Inst>
        static decltype(auto) value(Inst &inst)
        {
            if(inst.template hasType<T>())
            {
                return BaseVariant::template CurTypedDescr<T>::concrete(
                    inst.data());
            }
            throw std::logic_error("invalid variant type");
        }

        template<typename T, typename Inst>
        static decltype(auto) valueIf(Inst &inst)
        {
            auto makeResult = [&inst](){
                return &inst.template value<T>();
            };
            using R = decltype(makeResult());
            if(inst.template hasType<T>())
            {
                return makeResult();
            }
            return static_cast<R>(nullptr);
        }

    private:
        EphUniquePtr data_;
    };

    template<template<typename> class TypedDescr = BaseTypedDescr,
        template<template<typename> class> class Base = SkeletonConstVariant>
            requires (std::is_base_of_v<SkeletonConstVariant<TypedDescr>,
                Base<TypedDescr>>)
    class BaseConstRefVariant: public Base<TypedDescr>
    {
        template<template<typename> class T,
            template<template<typename> class> class B>
                requires (std::is_base_of_v<SkeletonVariant<T>,
                    B<T>>)
        friend class BaseVariant;

    public:
        template<typename T>
            requires (!std::is_base_of_v<SkeletonConstVariant<TypedDescr>, std::decay_t<T>>)
        explicit BaseConstRefVariant(T &&value)
            :Base<TypedDescr>(&BaseConstRefVariant::template typeDescr<T>()),
            data_(&value)
        {}

        // TODO: universal ref
        template<template<typename> class T,
            template<template<typename> class> class B>
        explicit BaseConstRefVariant(const BaseVariant<T, B> &that)
            :BaseConstRefVariant(that.descr(), that.constData())
        {}

        template<typename T>
        const T &value() const
        {
            if(BaseConstRefVariant::template hasType<T>())
            {
                return BaseConstRefVariant::template CurTypedDescr<T>::concrete(
                    BaseConstRefVariant::constData());
            }
            throw std::logic_error("invalid variant type");
        }

        template<typename T>
        const T *valueIf() const
        {
            if(BaseConstRefVariant::template hasType<T>())
            {
                return &value<T>();
            }
            return nullptr;
        }

    protected:
        BaseConstRefVariant(const typename BaseConstRefVariant::Descr &descr,
            typename BaseConstRefVariant::Descr::ConstEphPtr data)
            :Base<TypedDescr>(&descr), data_(data)
        {}

        typename BaseConstRefVariant::Descr::ConstEphPtr constData() const override
        {
            assert(data_);
            return data_;
        }

    private:
        typename BaseConstRefVariant::Descr::ConstEphPtr data_;
    };

    template<template<typename> class TypedDescr = BaseTypedDescr,
        template<template<typename> class> class Base = SkeletonVariant>
            requires (std::is_base_of_v<SkeletonVariant<TypedDescr>,
                Base<TypedDescr>>)
    class BaseRefVariant: public Base<TypedDescr>
    {
    public:
        template<typename T>
            requires (!std::is_base_of_v<SkeletonConstVariant<TypedDescr>,
                std::decay_t<T>>)
        explicit BaseRefVariant(T &value)
            :Base<TypedDescr>(&BaseRefVariant::template typeDescr<T>()),
            data_(&value)
        {}

        template<template<typename> class T,
            template<template<typename> class> class B>
        explicit BaseRefVariant(BaseVariant<T, B> &val)
            :BaseRefVariant(val.descr(), val.data())
        {}

        template<typename T>
        T &value() const
        {
            if(BaseRefVariant::template hasType<T>())
            {
                return BaseRefVariant::template CurTypedDescr<T>::concrete(
                    BaseRefVariant::data());
            }
            throw std::logic_error("invalid variant type");
        }

        template<typename T>
        T *valueIf() const
        {
            if(BaseRefVariant::template hasType<T>())
            {
                return &value<T>();
            }
            return nullptr;
        }

    protected:
        BaseRefVariant(const typename BaseRefVariant::Descr &descr,
            typename BaseRefVariant::Descr::EphPtr data)
            :Base<TypedDescr>(&descr), data_(data)
        {}

        typename BaseRefVariant::Descr::EphPtr data() const override
        {
            assert(data_);
            return data_;
        }
        typename BaseRefVariant::Descr::ConstEphPtr constData() const override
        {
            assert(data_);
            return data_;
        }

    private:
         typename BaseRefVariant::Descr::EphPtr data_;
    };

    using Variant = BaseVariant<>;
    using RefVariant = BaseRefVariant<>;
    using ConstRefVariant = BaseConstRefVariant<>;
}

#endif
