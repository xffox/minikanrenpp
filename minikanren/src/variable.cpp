#include "minikanren/variable.hpp"

#include "inner/id_allocator.hpp"

namespace minikanren
{
    Variable::Variable()
        :id_(inner::IDAllocator::allocate())
    {}
}
