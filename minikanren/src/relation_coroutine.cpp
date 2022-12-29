#include "minikanrenpp/relation_coroutine.hpp"

#include <utility>
#include <cassert>

#include "inner/manager.hpp"

namespace minikanrenpp
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
        PromisePtr self(&handle().promise(), RelationDeleter{});
        this->self_ = self;
        return RelationFuture{std::move(self)};
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

    void RelationPromise::apply(const OpCond &op)
    {
        assert(manager);
        manager->unify(self(), op.relation());
    }

    void RelationPromise::apply(const OpAnd &op)
    {
        assert(manager);
        manager->addAnd(self(), op.continuations());
    }

    void RelationPromise::apply(const OpOr &op)
    {
        assert(manager);
        manager->addOr(self(), op.getContinuations());
    }

    void RelationPromise::apply(PromisePtr promise)
    {
        assert(manager);
        manager->add(self(), std::move(promise));
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
