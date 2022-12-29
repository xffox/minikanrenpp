#ifndef MINIKANRENPP_RELATION_HPP
#define MINIKANRENPP_RELATION_HPP

#include <compare>
#include <string>
#include <optional>
#include <variant>
#include <cstddef>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <memory>
#include <ostream>

#include "minikanrenpp/variable.hpp"
#include "minikanrenpp/data/value.hpp"

namespace minikanrenpp
{
    class Relation
    {
    public:
        Relation(const data::Value &left, const data::Value &right)
            :left_(left), right_(right)
        {}

        const data::Value &left() const
        {
            return left_;
        }

        const data::Value &right() const
        {
            return right_;
        }

    private:
        data::Value left_;
        data::Value right_;
    };

    class ReifiedValue
    {
    public:
        explicit ReifiedValue(Variable var)
            :var(std::move(var))
        {}

        const Variable &idx() const
        {
            return var;
        }

        auto operator<=>(const ReifiedValue&) const = default;

    private:
        Variable var;
    };

    class RelationCandidate: public data::ValueStore
    {
    public:
        RelationCandidate() = default;
        RelationCandidate(const RelationCandidate &that);
        RelationCandidate &operator=(const RelationCandidate &that);
        RelationCandidate(RelationCandidate&&) = default;
        RelationCandidate &operator=(RelationCandidate&&) = default;

        bool unify(const Relation &relation);
        bool valid() const;

        std::optional<data::Value> result(const data::Value &query) const;

    protected:
        bool update(const data::ConstRefValue &left, const data::ConstRefValue &right) override;
        bool update(const Variable &left, const data::ConstRefValue &right) override;
        bool update(const Variable &left, const Variable &right) override;

        std::optional<data::Value> result(std::unordered_set<Variable> &seen,
            const Variable &var) const override;

    private:
        using VariableGroupValue = std::variant<ReifiedValue, data::Value>;

        struct VariableGroup
        {
            VariableGroupValue value;
            std::unordered_set<Variable> variables;
        };

    private:
        bool unify(const Variable &typedLeft, const data::Value &right);

        std::optional<data::Value> expand(const Variable &typedValue) const;

        VariableGroup &prepareVariable(const Variable &var);

        std::unordered_map<Variable, std::shared_ptr<VariableGroup>>
            cloneVariableGroups() const;

    private:
        bool valid_ = true;
        std::unordered_map<Variable, std::shared_ptr<VariableGroup>> variableGroups{};
    };
}

#endif
