#include "manager.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <iterator>
#include <cassert>

namespace minikanren::inner
{
    namespace
    {
        RelationFuture startCont(Cont<> cont)
        {
            co_await cont();
        }
    }

    RelationManager::RelationManager(Cont<> entry)
        :candidates(), futures(), deps()
    {
        auto promise = start(entry, 0, {});
        auto candID = append(RelationCandidate());
        addDependency(candID, promise.get());
        schedule(std::move(promise));
    }

    std::optional<RelationCandidate> RelationManager::result()
    {
        auto iter = std::find_if(std::begin(candidates), std::end(candidates),
            [this](auto &cand){
                return candDeps.find(&cand) == std::end(candDeps);
            });
        if(iter != std::end(candidates))
        {
            auto res = std::move(*iter);
            remove(&*iter);
            return res;
        }
        return {};
    }

    std::optional<PromisePtr> RelationManager::pick()
    {
        while(!futures.empty())
        {
            auto result = futures.top().promise;
            futures.pop();
            auto iter = deps.find(result.get());
            if(iter != end(deps) && !iter->second.empty())
            {
                return result;
            }
        }
        return {};
    }

    void RelationManager::unify(const PromisePtr &prom,
            const Relation &relation)
    {
        assert(prom);
        auto iter = deps.find(prom.get());
        if(iter != std::end(deps))
        {
            std::vector<CandidateID> invalidCandidates;
            for(auto cand : iter->second)
            {
                cand->unify(relation);
                if(!cand->valid())
                {
                    // Removing directly is tricky because of the iterators
                    // validity.
                    invalidCandidates.push_back(cand);
                }
            }
            for(auto cand : invalidCandidates)
            {
                remove(cand);
            }
        }
    }

    void RelationManager::addAnd(const PromisePtr &prom,
        const std::vector<Cont<>> &conts)
    {
        assert(prom);
        auto iter = deps.find(prom.get());
        if(iter != std::end(deps))
        {
            assert(!iter->second.empty());
            for(const auto &cont : conts)
            {
                auto contPromise = start(cont, prom->depth()+1, prom);
                for(auto cand : iter->second)
                {
                    addDependency(cand, contPromise.get());
                }
                schedule(std::move(contPromise));
            }
        }
    }

    void RelationManager::addOr(const PromisePtr &prom,
        const std::vector<Cont<>> &conts)
    {
        assert(prom);
        auto iter = deps.find(prom.get());
        if(iter != std::end(deps))
        {
            // TODO: optimize the copy
            const auto curDeps = iter->second;
            assert(!iter->second.empty());
            for(std::size_t i = 1; i < conts.size(); ++i)
            {
                auto contPromise = start(conts[i], prom->depth()+1, prom);
                for(auto cand : curDeps)
                {
                    auto newCandID = append(RelationCandidate(*cand));
                    auto depsIter = candDeps.find(cand);
                    if(depsIter != std::end(candDeps))
                    {
                        for(auto dep : depsIter->second)
                        {
                            addDependency(newCandID, dep);
                        }
                    }
                    addDependency(newCandID, contPromise.get());
                }
                schedule(std::move(contPromise));
            }
            if(!conts.empty())
            {
                auto contPromise = start(conts.front(), prom->depth()+1, prom);
                for(auto cand : curDeps)
                {
                    addDependency(cand, contPromise.get());
                }
                schedule(std::move(contPromise));
            }
        }
    }

    void RelationManager::add(const PromisePtr prom, PromisePtr call)
    {
        assert(prom);
        call->setManager(*this);
        call->setDepth(prom->depth()+1);
        call->setParent(prom);
        auto iter = deps.find(prom.get());
        if(iter != std::end(deps))
        {
            for(auto dep : iter->second)
            {
                addDependency(dep, call.get());
            }
            schedule(std::move(call));
        }
    }

    void RelationManager::finish(RelationPromise &prom)
    {
        removeDependencies(&prom);
    }

    void RelationManager::schedule(PromisePtr promise)
    {
        if(!promise->done())
        {
            futures.push({std::move(promise)});
        } else {
            finish(*promise);
        }
    }

    RelationManager::CandidateID RelationManager::append(
        RelationCandidate &&cand)
    {
        auto iter = candidates.insert(std::end(candidates), std::move(cand));
        return &*iter;
    }

    void RelationManager::remove(CandidateID candID)
    {
        removeDependencies(candID);
        auto iter = std::find_if(std::begin(candidates), std::end(candidates),
            [candID](const auto &cand){return &cand == candID;});
        if(iter != std::end(candidates))
        {
            candidates.erase(iter);
        }
    }

    void RelationManager::addDependency(CandidateID cand, DependencyID dep)
    {
        deps[dep].insert(cand);
        candDeps[cand].insert(dep);
    }

    void RelationManager::removeDependency(CandidateID cand, DependencyID dep)
    {
        removeDependencyForCandidate(cand, dep);
        removeDependencyForDependency(dep, cand);
    }

    void RelationManager::removeDependencies(DependencyID dep)
    {
        auto iter = deps.find(dep);
        if(iter != std::end(deps))
        {
            for(auto cand : iter->second)
            {
                removeDependencyForCandidate(cand, dep);
            }
            deps.erase(iter);
        }
    }

    void RelationManager::removeDependencies(CandidateID cand)
    {
        auto iter = candDeps.find(cand);
        if(iter != std::end(candDeps))
        {
            for(auto dep : iter->second)
            {
                removeDependencyForDependency(dep, cand);
            }
            candDeps.erase(iter);
        }
    }

    void RelationManager::removeDependencyForCandidate(
        CandidateID cand, DependencyID dep)
    {
        auto iter = candDeps.find(cand);
        if(iter != std::end(candDeps))
        {
            iter->second.erase(dep);
            if(iter->second.empty())
            {
                candDeps.erase(iter);
            }
        }
    }

    void RelationManager::removeDependencyForDependency(
        DependencyID dep, CandidateID cand)
    {
        auto iter = deps.find(dep);
        if(iter != std::end(deps))
        {
            iter->second.erase(cand);
            if(iter->second.empty())
            {
                deps.erase(iter);
            }
        }
    }

    PromisePtr RelationManager::start(Cont<> cont, std::size_t depth,
        PromisePtr parent)
    {
        auto fut = startCont(std::move(cont));
        auto promise = fut.promise();
        promise->setParent(std::move(parent));
        promise->setManager(*this);
        promise->setDepth(depth);
        return promise;
    }

    std::weak_ordering RelationManager::PrioritizedHandle::operator<=>(
        const PrioritizedHandle &that) const
    {
        return that.promise->depth() <=> promise->depth();
    }
}
