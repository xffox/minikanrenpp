#ifndef MINIKANRENPP_INNER_IDALLOCATOR_HPP
#define MINIKANRENPP_INNER_IDALLOCATOR_HPP

#include <cstddef>
#include <atomic>

namespace minikanrenpp::inner
{
    class IDAllocator
    {
    public:
        static std::size_t allocate()
        {
            static IDAllocator allocator;
            return allocator.allocateID();
        }

    private:
        IDAllocator()
        {}

        std::size_t allocateID()
        {
            return id.fetch_add(1);
        }

    private:
        std::atomic<std::size_t> id = 0;
    };
}

#endif
