#include <catch2/catch_test_macros.hpp>

#include <gdsb/experiment.h>

TEST_CASE("experiment")
{
    SECTION("out yields no exception")
    {
        std::vector<int> no_data;

        REQUIRE_NOTHROW(gdsb::out("no_data", std::begin(no_data), std::end(no_data)));
    }
}
