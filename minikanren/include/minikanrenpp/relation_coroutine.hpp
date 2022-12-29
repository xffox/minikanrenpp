#ifndef MINIKANRENPP_RELATIONCOROUTINE_HPP
#define MINIKANRENPP_RELATIONCOROUTINE_HPP

#include <cstddef>
#include <optional>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <coroutine>

#include "minikanrenpp/data/value.hpp"

namespace minikanrenpp
{
    namespace inner
    {
        class RelationManager;
    }

    class RelationFuture;

    class OpCond;
    class OpAnd;
    class OpOr;

    template<typename... Args>
    using Cont = std::function<RelationFuture(Args...)>;

    class RelationPromise;

    using RelationHandle = std::coroutine_handle<RelationPromise>;
    using PromisePtr = std::shared_ptr<RelationPromise>;

    class RelationPromise
    {
    public:
        RelationFuture get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void unhandled_exception();

        RelationHandle handle();
        PromisePtr self()
        {
            return self_.lock();
        }

        void setManager(inner::RelationManager &manager)
        {
            this->manager = &manager;
        }

        void setDepth(std::size_t depth)
        {
            depth_ = depth;
        }
        std::size_t depth() const
        {
            return depth_;
        }

        void setParent(PromisePtr parent)
        {
            this->parent = std::move(parent);
        }

        void apply(const OpCond &op);
        void apply(const OpAnd &op);
        void apply(const OpOr &op);
        void apply(PromisePtr promise);

        void run();
        bool done();
        void finish();

    private:
        inner::RelationManager *manager = nullptr;
        std::size_t depth_ = 0;
        std::optional<data::Value> result{};
        std::weak_ptr<RelationPromise> self_{};
        PromisePtr parent{};
    };

    class RelationFuture
    {
        friend class RelationPromise;

    public:
        using promise_type = RelationPromise;

    public:
        const PromisePtr &promise() const;

        constexpr bool await_ready() const noexcept {return false;}
        constexpr void await_resume() const noexcept {}
        void await_suspend(
            std::coroutine_handle<RelationPromise> handle);

    private:
        RelationFuture(PromisePtr &&promise);

    private:
        PromisePtr promise_;
    };
}

#endif
