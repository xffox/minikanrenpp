#ifndef MINIKANREN_RUN_HPP
#define MINIKANREN_RUN_HPP

#include <cstddef>
#include <type_traits>
#include <optional>
#include <iterator>
#include <utility>
#include <memory>

#include "minikanren/variable.hpp"
#include "minikanren/relation_coroutine.hpp"
#include "minikanren/data/value.hpp"

namespace minikanren
{
    namespace inner
    {
        class RelationManager;
    }

    class Run;

    class RunIterator
    {
        friend class Run;

        friend bool operator==(const RunIterator &iter,
            std::default_sentinel_t)
        {
            return !static_cast<bool>(iter.maybeResult);
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = data::Value;

    public:
        const data::Value &operator*() const;

        RunIterator &operator++();
        RunIterator operator++(int);

    private:
        RunIterator(Run &run);

    private:
        Run *run;
        std::optional<data::Value> maybeResult{};
    };

    class Run
    {
        friend class RunIterator;

    public:
        template<typename... Args>
        Run(data::Value query, Cont<
                typename std::pair<std::decay_t<Args>, int>::first_type...> entry,
            Args &&...args)
            :Run([entry, ...vs = std::forward<Args>(args)](){
                    return entry(vs...);
                }, std::move(query))
        {}

        Run(Run&&) = default;
        Run(const Run&) = delete;
        ~Run();

        auto begin()
        {
            return RunIterator{*this};
        }

        auto end()
        {
            return std::default_sentinel;
        }

    private:
        Run(Cont<> cont, data::Value query);

        std::optional<data::Value> advance();

    private:
        data::Value query;
        std::shared_ptr<inner::RelationManager> manager;
    };
}

#endif
