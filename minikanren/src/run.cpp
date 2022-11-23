#include "minikanren/run.hpp"

#include <utility>
#include <cassert>

#include "inner/manager.hpp"

namespace minikanren
{
    RunIterator::RunIterator(Run &run)
        :run(&run), maybeResult(this->run->advance())
    {}

    Run::~Run() = default;

    const data::Value &RunIterator::operator*() const
    {
        return *maybeResult;
    }

    RunIterator &RunIterator::operator++()
    {
        assert(run);
        maybeResult = run->advance();
        return *this;
    }

    RunIterator RunIterator::operator++(int)
    {
        auto iter = *this;
        ++(*this);
        return iter;
    }

    Run::Run(Cont<> cont, data::Value query)
        :query(std::move(query)),
        manager(new inner::RelationManager(std::move(cont)))
    {}

    std::optional<data::Value> Run::advance()
    {
        assert(manager);
        std::optional<data::Value> result{};
        while(true)
        {
            auto res = manager->result();
            if(res)
            {
                result = res->result(query);
                assert(result);
                break;
            }
            auto fut = manager->pick();
            if(!fut)
            {
                manager.reset();
                break;
            }
            (*fut)->run();
            manager->schedule(std::move(*fut));
        }
        return result;
    }
}
