#ifndef MINIKANREN_INNER_MANAGER_HPP
#define MINIKANREN_INNER_MANAGER_HPP

#include <cstddef>
#include <list>
#include <compare>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <initializer_list>
#include <optional>
#include <utility>
#include <vector>
#include <memory>

#include "minikanren/relation_coroutine.hpp"
#include "minikanren/relation.hpp"
#include "minikanren/operation.hpp"

namespace minikanren::inner
{
    class RelationManager
    {
    public:
        RelationManager(Cont<> entry);

        std::optional<RelationCandidate> result();

        std::optional<PromisePtr> pick();
        void schedule(PromisePtr promise);

        void apply(RelationPromise &prom,
            const OpCond &op);
        void apply(RelationPromise &prom,
            const OpAnd &op);
        void apply(RelationPromise &prom,
            const OpOr &op);
        void apply(RelationPromise &prom,
            PromisePtr call);
        void finish(RelationPromise &prom);

    private:
        using CandidateList = std::list<RelationCandidate>;
        using CandidateID = RelationCandidate*;

        struct PrioritizedHandle
        {
            std::weak_ordering operator<=>(
                const PrioritizedHandle &that) const;

            PromisePtr promise;
        };

    private:
        CandidateID append(RelationCandidate &&cand);
        void remove(CandidateID candID);

        void addDependency(CandidateID cand, DependencyID dep);
        void removeDependency(CandidateID cand, DependencyID dep);
        void removeDependencies(DependencyID dep);
        void removeDependencies(CandidateID cand);
        void removeDependencyForCandidate(CandidateID cand, DependencyID dep);
        void removeDependencyForDependency(DependencyID dep, CandidateID cand);

        PromisePtr start(Cont<> cont, std::size_t depth);

    private:
        CandidateList candidates;
        std::priority_queue<PrioritizedHandle> futures;
        std::unordered_map<DependencyID, std::unordered_set<CandidateID>> deps;
        std::unordered_map<CandidateID, std::unordered_set<DependencyID>> candDeps;
    };
}

#endif
