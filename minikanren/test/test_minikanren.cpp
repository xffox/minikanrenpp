#include <algorithm>
#include <vector>
#include <iterator>

#include <catch2/catch.hpp>

#include <minikanrenpp/minikanren.hpp>
#include <minikanrenpp/data/list.hpp>

using namespace minikanrenpp;

namespace
{
    RelationFuture append(data::Value x, data::Value v1, data::Value v2)
    {
        auto append = [](auto append, auto r, auto v1, auto v2)->RelationFuture{
            co_await conde({
                [r, v1, v2]()->RelationFuture{
                    co_await eq(v1, data::Nil{});
                    co_await eq(r, v2);
                },
                [append, r, v1, v2]()->RelationFuture{
                    co_await fresh({[append, r, v1, v2](
                                auto h, auto t, auto n)->RelationFuture{
                            co_await eq(v1, data::Cons(h, t));
                            co_await fresh({append}, append, n, t, v2);
                            co_await eq(r, data::Cons(h, n));
                        }}, Variable(), Variable(), Variable());
                }
            });
        };
        co_await fresh({append}, append, x, v1, v2);
    }

    RelationFuture recurse(Variable v)
    {
        co_await eq(v, data::Cons(0, v));
    }

    bool haveSameValues(
        const std::vector<data::Value> &left, const std::vector<data::Value> &right)
    {
        if(left.size() != right.size())
        {
            return false;
        }
        for(const auto &val : left)
        {
            if(std::find(begin(right), end(right), val) == end(right))
            {
                return false;
            }
        }
        return true;
    }
}

TEST_CASE("append data::list result")
{
    std::vector<data::Value> act;
    for(Variable r; auto &&res : Run(r, append, r,
            data::list(1, 2, 3, 7), data::list(4, 5, 6, 42, 97)))
    {
        act.push_back(res);
    }
    REQUIRE(act == std::vector<data::Value>{data::list(1, 2, 3, 7, 4, 5, 6, 42, 97)});
}

TEST_CASE("append data::list argument")
{
    std::vector<data::Value> act;
    for(Variable r; auto &&res : Run(r, append,
            data::list(1, 2, 3, 7, 4, 8), data::list(1, 2, 3, 7), r))
    {
        act.push_back(res);
    }
    REQUIRE(act == std::vector<data::Value>{data::list(4, 8)});
}

TEST_CASE("append argument combinations")
{
    std::vector<data::Value> act;
    for(Variable a, b; auto &&res : Run(data::list(a, b), append,
            data::list(4, 5, 1, 3, 9), a, b))
    {
        act.push_back(res);
    }
    REQUIRE(haveSameValues(act,
            std::vector<data::Value>{
                data::list(data::list(), data::list(4, 5, 1, 3, 9)),
                data::list(data::list(4), data::list(5, 1, 3, 9)),
                data::list(data::list(4, 5), data::list(1, 3, 9)),
                data::list(data::list(4, 5, 1), data::list(3, 9)),
                data::list(data::list(4, 5, 1, 3), data::list(9)),
                data::list(data::list(4, 5, 1, 3, 9), data::list()),
            }));
}

TEST_CASE("recursive")
{
    std::vector<data::Value> act;
    for(Variable r; auto &&res : Run(r, recurse, r))
    {
        act.push_back(res);
    }
    Variable result;
    Run unify(result, [](data::Value left, data::Value right)->RelationFuture{
            co_await eq(left, right);
        }, act, std::vector<data::Value>{data::Cons(0, data::RecursiveValue{Variable()})});
    bool first = false;
    for([[maybe_unused]] auto &&val : unify)
    {
        if(first)
        {
            FAIL();
        }
        first = true;
    }
    REQUIRE(first);
}
