#ifndef MINIKANREN_INNER_MANAGER_HPP
#define MINIKANREN_INNER_MANAGER_HPP

#include <cstddef>
#include <list>
#include <compare>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <initializer_list>
#include <optional>
#include <utility>
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

        void unify(const PromisePtr &prom, const Relation &relation);
        void addAnd(const PromisePtr &prom, const std::vector<Cont<>> &conts);
        void addOr(const PromisePtr &prom, const std::vector<Cont<>> &conts);
        void add(const PromisePtr prom, PromisePtr call);

    private:
        using CandidateList = std::list<RelationCandidate>;
        using CandidateID = RelationCandidate*;
        using DependencyID = const RelationPromise*;

        struct PrioritizedHandle
        {
            std::weak_ordering operator<=>(
                const PrioritizedHandle &that) const;

            PromisePtr promise;
        };

    private:
        void finish(RelationPromise &prom);
        CandidateID append(RelationCandidate &&cand);
        void remove(CandidateID candID);

        void addDependency(CandidateID cand, DependencyID dep);
        void removeDependency(CandidateID cand, DependencyID dep);
        void removeDependencies(DependencyID dep);
        void removeDependencies(CandidateID cand);
        void removeDependencyForCandidate(CandidateID cand, DependencyID dep);
        void removeDependencyForDependency(DependencyID dep, CandidateID cand);

        PromisePtr start(Cont<> cont, std::size_t depth, PromisePtr parent);

    private:
        CandidateList candidates;
        std::priority_queue<PrioritizedHandle> futures;
        std::unordered_map<DependencyID, std::unordered_set<CandidateID>> deps;
        std::unordered_map<CandidateID, std::unordered_set<DependencyID>> candDeps;
    };
}

#endif
