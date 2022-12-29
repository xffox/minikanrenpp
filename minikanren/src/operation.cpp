#include "minikanrenpp/operation.hpp"

namespace minikanrenpp
{
    OpCond eq(const data::Value &left, const data::Value &right)
    {
        return OpCond(Relation(left, right));
    }
}
