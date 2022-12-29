#ifndef MINIKANRENPP_DATA_LIST_HPP
#define MINIKANRENPP_DATA_LIST_HPP

#include <algorithm>
#include <type_traits>
#include <ranges>
#include <array>
#include <functional>
#include <utility>
#include <iterator>
#include <ostream>

#include "minikanrenpp/data/value.hpp"
#include "minikanrenpp/data/fragmentable.hpp"

namespace minikanrenpp::data
{
    class Nil
    {
    public:
        using value_type = Value;

    public:
        Nil() = default;

        template<std::ranges::range R>
        Nil(const R&)
        {}

        bool operator==(const Nil&) const = default;

        auto begin() const
        {
            return std::ranges::begin(vs);
        }
        auto end() const
        {
            return std::ranges::end(vs);
        }

    private:
        std::array<Value, 0> vs{};
    };

    class Cons
    {
    public:
        using value_type = Value;

    public:
        Cons(Value car, Value cdr)
            :values{std::move(car), std::move(cdr)}
        {}

        template<typename R>
            requires (std::ranges::range<std::decay_t<R>>)
        explicit Cons(R &&vs)
            :values{Nil{}, Nil{}}
        {
            auto beginIter = std::ranges::begin(vs);
            auto endIter = std::ranges::begin(vs) +
                std::min(values.size(), vs.size());
            auto outIter = std::ranges::begin(values);
            if constexpr(std::is_rvalue_reference_v<R>)
            {
                std::move(beginIter, endIter, outIter);
            }
            else
            {
                std::copy(beginIter, endIter, outIter);
            }
        }

        const Value &car() const
        {
            return values[0];
        }
        const Value &cdr() const
        {
            return values[1];
        }

        bool operator==(const Cons&) const = default;

        auto begin() const
        {
            return std::ranges::begin(values);
        }
        auto end() const
        {
            return std::ranges::end(values);
        }

    private:
        std::array<Value, 2> values;
    };

    inline Value list()
    {
        return Nil{};
    }
    template<typename Arg, typename... Args>
    Value list(Arg &&arg, Args &&...args)
    {
        return Cons{
            std::forward<Arg>(arg),
            list(std::forward<Args>(args)...)
        };
    }
}

static_assert(minikanrenpp::data::Fragmentable<minikanrenpp::data::Cons>);
static_assert(minikanrenpp::data::Fragmentable<minikanrenpp::data::Nil>);

#endif
