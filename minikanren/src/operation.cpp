#include "minikanren/operation.hpp"

namespace minikanren
{
    OpCond eq(const data::Value &left, const data::Value &right)
    {
        return OpCond(Relation(left, right));
    }
}
