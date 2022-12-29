#include <utility>

#include <catch2/catch.hpp>

#include <minikanrenpp/xvariant/variant.hpp>

using namespace minikanrenpp::xvariant;

TEST_CASE("create")
{
    constexpr int val = 42;
    Variant v(val);
    REQUIRE(v.hasType<int>());
    REQUIRE(v.value<int>() == val);
}

TEST_CASE("assignment")
{
    constexpr int val = 42;
    constexpr auto nextVal = val+3;
    Variant v(val);
    v.value<int>() = nextVal;
    REQUIRE(v.hasType<int>());
    REQUIRE(v.value<int>() == nextVal);
}

TEST_CASE("copy construction")
{
    constexpr int val = 84;
    Variant v(val);
    Variant tgt(v);
    REQUIRE(tgt.hasType<int>());
    REQUIRE(tgt.value<int>() == val);
}

TEST_CASE("move construction")
{
    constexpr int val = 17;
    Variant v(val);
    Variant tgt(std::move(v));
    REQUIRE(tgt.hasType<int>());
    REQUIRE(tgt.value<int>() == val);
}

TEST_CASE("copy assignment")
{
    constexpr int val = 103;
    Variant v(val);
    Variant tgt{TypeTag<char>{}};
    tgt = v;
    REQUIRE(tgt.hasType<int>());
    REQUIRE(tgt.value<int>() == val);
    REQUIRE(tgt == v);
}

TEST_CASE("move assignment")
{
    constexpr int val = 97;
    Variant v(val);
    Variant tgt{TypeTag<char>{}};
    tgt = std::move(v);
    REQUIRE(tgt.hasType<int>());
    REQUIRE(tgt.value<int>() == val);
}
