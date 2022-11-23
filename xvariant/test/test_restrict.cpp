#include <string>
#include <utility>
#include <sstream>

#include <catch2/catch.hpp>

#include <xvariant/variant.hpp>
#include <xvariant/descr.hpp>

using namespace xvariant;

namespace
{
    class ShowDescr: public Description
    {
    public:
        virtual std::string show(const ConstEphPtr &val) const = 0;
    };

    template<typename T>
    class TypedShowDescr: public TypedDescription<T, ShowDescr>
    {
    public:
        std::string show(
            const typename TypedShowDescr::ConstEphPtr &val) const override
        {
            std::stringstream ss;
            ss<<TypedShowDescr::concrete(val);
            return std::move(ss).str();
        }
    };

    class ShowVariant: public BaseVariant<TypedShowDescr>
    {
    public:
        using BaseVariant::BaseVariant;

        std::string show() const
        {
            return descr().show(data());
        }
    };
}

TEST_CASE("show integral")
{
    ShowVariant val(42);
    REQUIRE(val.show() == "42");
}

TEST_CASE("show string")
{
    const std::string str("Mordor");
    ShowVariant val(str);
    REQUIRE(val.show() == str);
}
