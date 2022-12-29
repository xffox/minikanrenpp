#ifndef MINIKANRENPP_DATA_RECURSIVEVALUE_HPP
#define MINIKANRENPP_DATA_RECURSIVEVALUE_HPP

#include <ranges>
#include <type_traits>
#include <array>
#include <utility>
#include <stdexcept>
#include <ostream>

#include "minikanrenpp/variable.hpp"
#include "minikanrenpp/data/fragmentable.hpp"

namespace minikanrenpp::data
{
    class RecursiveValue
    {
    public:
        using value_type = Variable;

    private:
        template<typename R>
            requires (std::ranges::range<std::decay_t<R>>)
        decltype(auto) extractValue(R &&vs)
        {
            auto iter = std::ranges::begin(vs);
            auto endIter = std::ranges::end(vs);
            if(iter == endIter)
            {
                throw std::invalid_argument("invalid arguments");
            }
            auto extract = [](auto &&val) -> decltype(auto) {
                if constexpr(std::is_rvalue_reference_v<R>)
                {
                    return std::move(val);
                }
                else
                {
                    return val;
                }
            };
            auto result = extract(*iter);
            ++iter;
            if(iter != endIter)
            {
                throw std::invalid_argument("invalid arguments");
            }
            return result;
        }

    public:
        explicit RecursiveValue(Variable var)
            :var{std::move(var)}
        {}

        template<typename R>
            requires (std::ranges::range<std::decay_t<R>>)
        explicit RecursiveValue(R &&vs)
            :var{extractValue(std::forward<R>(vs))}
        {}

        bool operator==(const RecursiveValue&) const = default;

        auto begin() const
        {
            return std::ranges::begin(var);
        }
        auto end() const
        {
            return std::ranges::end(var);
        }

    private:
        std::array<Variable, 1> var;
    };
}

static_assert(minikanrenpp::data::Fragmentable<minikanrenpp::data::RecursiveValue>);

#endif
