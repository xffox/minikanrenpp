#include "minikanrenpp/relation.hpp"

#include <algorithm>
#include <any>
#include <iterator>
#include <optional>
#include <functional>
#include <utility>
#include <stdexcept>
#include <cassert>

#include "minikanrenpp/data/recursive_value.hpp"

namespace minikanrenpp
{
    namespace
    {
        template<typename C>
        typename C::iterator merge(C &groups, typename C::iterator left,
            typename C::iterator right)
        {
            auto &leftGroup = left->second;
            auto rightGroup = right->second;
            leftGroup->variables.insert(
                begin(rightGroup->variables),
                end(rightGroup->variables));
            for(const auto &var : rightGroup->variables)
            {
                auto iter = groups.find(var);
                assert(iter != end(groups));
                iter->second = leftGroup;
            }
            return left;
        }

        template<typename C>
        void append(C &groups, typename C::iterator iter,
            const Variable &var)
        {
            auto &group = iter->second;
            group->variables.insert(var);
            groups.insert_or_assign(var, group);
        }
    }

    RelationCandidate::RelationCandidate(const RelationCandidate &that)
        :valid_(that.valid_),
        variableGroups(that.cloneVariableGroups())
    {}

    RelationCandidate &RelationCandidate::operator=(const RelationCandidate &that)
    {
        return (*this = RelationCandidate(that));
    }

    bool RelationCandidate::unify(const Relation &relation)
    {
        assert(valid_);
        return relation.left().unify(*this, data::ConstRefValue{relation.right()});
    }

    bool RelationCandidate::valid() const
    {
        return valid_;
    }

    std::optional<data::Value> RelationCandidate::result(
        const data::Value &query) const
    {
        std::unordered_set<Variable> seen;
        return query.expand(seen, *this);
    }

    bool RelationCandidate::update(
        const data::ConstRefValue &left, const data::ConstRefValue &right)
    {
        if(left != right)
        {
            valid_ = false;
        }
        return valid_;
    }

    bool RelationCandidate::update(
        const Variable &left, const data::ConstRefValue &right)
    {
        struct ValueVisitor
        {
            bool operator()(ReifiedValue) const
            {
                group->value = data::Value(*that);
                return true;
            }
            bool operator()(const data::Value &val) const
            {
                return val.unify(*candidate, *that);
            }

            RelationCandidate *candidate;
            VariableGroup *group;
            const data::ConstRefValue *that;
        };
        auto &group = prepareVariable(left);
        return std::visit(ValueVisitor{this, &group, &right}, group.value);
    }

    bool RelationCandidate::update(const Variable &left, const Variable &right)
    {
        auto leftIter = variableGroups.find(left);
        auto rightIter = variableGroups.find(right);
        if(leftIter != end(variableGroups) && rightIter != end(variableGroups))
        {
            struct ValuesVisitor
            {
                bool operator()(const ReifiedValue &left, const ReifiedValue &right) const
                {
                    if(left > right)
                    {
                        return (*this)(right, left);
                    }
                    merge(candidate->variableGroups, leftIter, rightIter);
                    return true;
                }
                bool operator()(const data::Value &left, const data::Value &right) const
                {
                    if(left.unify(*candidate, data::ConstRefValue{right}))
                    {
                        merge(candidate->variableGroups, leftIter, rightIter);
                        return true;
                    }
                    return false;
                }
                bool operator()(const ReifiedValue&, data::Value right) const
                {
                    merge(candidate->variableGroups,
                        leftIter, rightIter)->second->value = std::move(right);
                    return true;
                }
                bool operator()(const data::Value &left, const ReifiedValue &right) const
                {
                    return (*this)(right, left);
                }

                RelationCandidate *candidate;
                decltype(leftIter) leftIter;
                decltype(rightIter) rightIter;
            };
            return std::visit(ValuesVisitor{this, leftIter, rightIter},
                leftIter->second->value,
                rightIter->second->value);
        }
        else if(leftIter != end(variableGroups))
        {
            append(variableGroups, leftIter, right);
            return true;
        }
        else if(rightIter != end(variableGroups))
        {
            append(variableGroups, rightIter, left);
            return true;
        }
        else
        {
            prepareVariable(left);
            auto iter = variableGroups.find(left);
            assert(iter != end(variableGroups));
            append(variableGroups, iter, right);
            return true;
        }
    }

    std::optional<data::Value> RelationCandidate::result(
        std::unordered_set<Variable> &seen,
        const Variable &var) const
    {
        if(!seen.insert(var).second)
        {
            return data::RecursiveValue{var};
        }
        auto iter = variableGroups.find(var);
        if(iter != end(variableGroups))
        {
            struct ValueVisitor
            {
                std::optional<data::Value> operator()(ReifiedValue idx) const
                {
                    return data::Value{idx};
                }
                std::optional<data::Value> operator()(const data::Value &val) const
                {
                    return val.expand(*seen, *inst);
                }

                const RelationCandidate *inst;
                std::unordered_set<Variable> *seen;
            };
            return std::visit(ValueVisitor{this, &seen}, iter->second->value);
        }
        return ReifiedValue{var};
    }

    RelationCandidate::VariableGroup &RelationCandidate::prepareVariable(
        const Variable &var)
    {
        auto iter = variableGroups.find(var);
        if(iter == end(variableGroups))
        {
            iter = variableGroups.insert({var,
                std::shared_ptr<VariableGroup>(
                    new VariableGroup{VariableGroupValue(ReifiedValue{Variable{}}),
                        std::unordered_set<Variable>{var}})}).first;
        }
        return *iter->second;
    }

    std::unordered_map<Variable, std::shared_ptr<RelationCandidate::VariableGroup>>
        RelationCandidate::cloneVariableGroups() const
    {
        std::unordered_map<Variable, std::shared_ptr<RelationCandidate::VariableGroup>> result;
        std::transform(begin(variableGroups), end(variableGroups),
            std::inserter(result, end(result)),
            [](const auto &p){
                return std::make_pair(p.first,
                    std::make_shared<VariableGroup>(*p.second));
            });
        return result;
    }
}
