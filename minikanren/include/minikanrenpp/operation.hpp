#ifndef MINIKANRENPP_OPERATION_HPP
#define MINIKANRENPP_OPERATION_HPP

#include <algorithm>
#include <type_traits>
#include <coroutine>
#include <initializer_list>
#include <vector>
#include <iterator>
#include <utility>

#include "minikanrenpp/relation.hpp"
#include "minikanrenpp/relation_coroutine.hpp"

namespace minikanrenpp
{
    class RelationPromise;

    template<typename Op>
    class OpAwaiter
    {
    public:
        constexpr bool await_ready() const noexcept {return false;}
        constexpr void await_resume() const noexcept {}
        void await_suspend(
            std::coroutine_handle<RelationPromise> handle);
    };

    class OpCond: public OpAwaiter<OpCond>
    {
    public:
        OpCond(const Relation &relation)
            :relation_(relation)
        {}

        const Relation &relation() const
        {
            return relation_;
        }

    private:
        Relation relation_;
    };

    class OpAnd: public OpAwaiter<OpAnd>
    {
    public:
        template<typename TT>
        OpAnd(TT &&conts)
            :conts(std::forward<TT>(conts))
        {}
        OpAnd(std::initializer_list<Cont<>> conts)
            :OpAnd(std::vector<Cont<>>(conts))
        {}

        const std::vector<Cont<>> &continuations() const
        {
            return conts;
        }

    private:
        std::vector<Cont<>> conts;
    };

    class OpOr: public OpAwaiter<OpOr>
    {
    public:
        template<typename TT>
        OpOr(TT &&conts)
            :conts(std::forward<TT>(conts))
        {}
        OpOr(std::initializer_list<Cont<>> conts)
            :conts(std::vector<Cont<>>(conts))
        {}

        const std::vector<Cont<>> &getContinuations() const
        {
            return conts;
        }

    private:
        std::vector<Cont<>> conts;
    };

    template<typename Op>
    void OpAwaiter<Op>::await_suspend(
        std::coroutine_handle<RelationPromise> handle)
    {
        auto &op = static_cast<Op&>(*this);
        auto &promise = handle.promise();
        promise.apply(op);
    }

    template<typename Var, typename... Vars>
    OpAnd fresh(std::initializer_list<
            Cont<typename std::pair<std::decay_t<Var>, int>::first_type,
                typename std::pair<std::decay_t<Vars>, int>::first_type...>> conts,
        Var var, Vars ...vars);
    inline OpAnd fresh(std::initializer_list<Cont<>> conts);

    OpCond eq(const data::Value &left, const data::Value &right);

    namespace inner
    {
        template<typename Op, typename... Vars>
        Op makeOperation(
            std::initializer_list<
            Cont<typename std::pair<std::decay_t<Vars>, int>::first_type...>> conts,
            Vars ...vars)
        {
            std::vector<Cont<>> ops;
            ops.reserve(conts.size());
            std::transform(std::begin(conts), std::end(conts),
                std::back_inserter(ops),
                [&vars...](const auto &cont){
                    return [cont, vars...]()->RelationFuture{
                        return cont(vars...);
                    };
                });
            return Op(std::move(ops));
        }
    }

    template<typename Var, typename... Vars>
    OpAnd fresh(std::initializer_list<
            Cont<typename std::pair<std::decay_t<Var>, int>::first_type,
                typename std::pair<std::decay_t<Vars>, int>::first_type...>> conts,
        Var var, Vars ...vars)
    {
        return inner::makeOperation<OpAnd>(conts, var, vars...);
    }

    inline OpAnd fresh(std::initializer_list<Cont<>> conts)
    {
        return OpAnd(std::move(conts));
    }

    template<typename... Vars>
    OpOr conde(std::initializer_list<
            Cont<typename std::pair<std::decay_t<Vars>, int>::first_type...>> conts,
        Vars ...vars)
    {
        return inner::makeOperation<OpOr>(conts, vars...);
    }
}

#endif
