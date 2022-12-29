#include "minikanrenpp/variable.hpp"

#include "inner/id_allocator.hpp"

namespace minikanrenpp
{
    Variable::Variable()
        :id_(inner::IDAllocator::allocate())
    {}
}
