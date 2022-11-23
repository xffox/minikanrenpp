#include <catch2/catch.hpp>

#include <xvariant/variant.hpp>

using namespace xvariant;

TEST_CASE("create ref")
{
    int val = 42;
    RefVariant ref(val);
    REQUIRE(ref.hasType<decltype(val)>());
    REQUIRE(ref.value<decltype(val)>() == val);
    REQUIRE(*ref.valueIf<decltype(val)>() == val);
}

TEST_CASE("modify ref")
{
    int val = 42;
    const int expVal = val + 3;
    RefVariant ref(val);
    ref.value<decltype(val)>() = expVal;
    REQUIRE(val == expVal);
}

TEST_CASE("modify ref value")
{
    int val = 42;
    const int expVal = val + 3;
    RefVariant ref(val);
    val = expVal;
    REQUIRE(ref.value<decltype(val)>() == expVal);
}

TEST_CASE("create const ref")
{
    int val = 42;
    ConstRefVariant ref(val);
    REQUIRE(ref.hasType<decltype(val)>());
    REQUIRE(ref.value<decltype(val)>() == val);
    REQUIRE(*ref.valueIf<decltype(val)>() == val);
}
