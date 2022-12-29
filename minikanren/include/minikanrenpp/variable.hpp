#ifndef MINIKANRENPP_VARIABLE_HPP
#define MINIKANRENPP_VARIABLE_HPP

#include <cstddef>
#include <functional>
#include <ostream>

namespace minikanrenpp
{
    class Variable
    {
    public:
        explicit Variable();

        std::size_t id() const
        {
            return id_;
        }

        auto operator<=>(const Variable&) const = default;

        friend std::ostream &operator<<(std::ostream &stream, const Variable &var)
        {
            return stream<<"var{"<<var.id_<<"}";
        }

    private:
        std::size_t id_;
    };
}

template<>
struct std::hash<minikanrenpp::Variable>
{
    std::size_t operator()(const minikanrenpp::Variable &var) const noexcept
    {
        return std::hash<std::size_t>{}(var.id());
    }
};

#endif
