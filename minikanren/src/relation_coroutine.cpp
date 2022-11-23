#include "minikanren/relation_coroutine.hpp"

#include <utility>
#include <cassert>

#include "inner/manager.hpp"

namespace minikanren
{
    namespace
    {
        struct RelationDeleter
        {
            void operator()(RelationPromise *promise) const
            {
                promise->finish();
            }
        };

    }

    RelationFuture RelationPromise::get_return_object()
    {
        return RelationFuture{PromisePtr(&handle().promise(),
            RelationDeleter{})};
    }

    std::suspend_always RelationPromise::initial_suspend()
    {
        return {};
    }

    std::suspend_always RelationPromise::final_suspend() noexcept
    {
        return {};
    }

    void RelationPromise::unhandled_exception()
    {}

    RelationHandle RelationPromise::handle()
    {
        return RelationHandle::from_promise(*this);
    }

    void RelationPromise::storeCont(std::unique_ptr<Cont<>> &&cont)
    {
        storedCont = std::move(cont);
    }

    void RelationPromise::apply(const OpCond &op)
    {
        assert(manager);
        manager->apply(*this, op);
    }

    void RelationPromise::apply(const OpAnd &op)
    {
        assert(manager);
        manager->apply(*this, op);
    }

    void RelationPromise::apply(const OpOr &op)
    {
        assert(manager);
        manager->apply(*this, op);
    }

    void RelationPromise::apply(PromisePtr promise)
    {
        assert(manager);
        manager->apply(*this, std::move(promise));
    }

    void RelationPromise::run()
    {
        RelationHandle::from_promise(*this)();
    }

    bool RelationPromise::done()
    {
        return RelationHandle::from_promise(*this).done();
    }

    void RelationPromise::finish()
    {
        assert(manager);
        manager->finish(*this);
        RelationHandle::from_promise(*this).destroy();
    }

    RelationFuture::RelationFuture(PromisePtr &&promise)
        :promise_(std::move(promise))
    {}

    const PromisePtr &RelationFuture::promise() const
    {
        return promise_;
    }

    void RelationFuture::await_suspend(
        std::coroutine_handle<RelationPromise> handle)
    {
        handle.promise().apply(promise_);
    }
}
