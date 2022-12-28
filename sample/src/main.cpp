#include <cstddef>
#include <string>
#include <variant>
#include <vector>
#include <ranges>
#include <iostream>

#include <xvariant/variant.hpp>
#include <minikanren/minikanren.hpp>
#include <minikanren/data/list.hpp>

using namespace minikanren;

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
                    co_await fresh({[append, r, v1, v2](auto h, auto t, auto n)->RelationFuture{
                            co_await eq(v1, data::Cons(h, t));
                            co_await fresh({append}, append, n, t, v2);
                            co_await eq(r, data::Cons(h, n));
                        }}, Variable(), Variable(), Variable());
                }
            });
        };
        co_await fresh({append}, append, x, v1, v2);
    }

    RelationFuture mult(Variable v)
    {
        co_await conde({
            [&v]()->RelationFuture{
                co_await eq(v, 42);
            },
            [&v]()->RelationFuture{
                co_await eq(v, 43);
            },
        });
    }

    RelationFuture vect(Variable v)
    {
        co_await eq(std::vector<data::Value>{1, 2, v}, v);
    }

    std::string formatList(const data::Value &val)
    {
        if(const auto *nil = val.valueIf<data::Nil>())
        {
            return "nil";
        }
        if(const auto *cons = val.valueIf<data::Cons>())
        {
            return "(" + formatList(cons->car()) + " " + formatList(cons->cdr()) + ")";
        }
        if(const auto *v = val.valueIf<int>())
        {
            return std::to_string(*v);
        }
        return "?";
    }
}

int main()
{
    const auto result = data::list(1, 2, 3);
    std::cout<<"results:"<<std::endl;
    for(Variable a, b; auto &&res : Run(data::list(a, b), append, result, a, b))
    {
        std::cout<<formatList(res)<<std::endl;
    }
    return 0;
}
