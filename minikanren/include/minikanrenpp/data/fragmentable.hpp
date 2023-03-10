#ifndef MINIKANRENPP_DATA_FRAGMENTABLE_HPP
#define MINIKANRENPP_DATA_FRAGMENTABLE_HPP

#include <ranges>
#include <initializer_list>

namespace minikanrenpp::data
{
    template<typename T>
    concept Fragmentable = std::ranges::range<T> &&
        requires(std::initializer_list<typename T::value_type> vs)
    {
        { T(vs) };
    };

}

#endif
