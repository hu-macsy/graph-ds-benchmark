#include <catch2/catch_test_macros.hpp>

#include <gdsb/experiment.h>

#include <sstream>

TEST_CASE("experiment")
{
    SECTION("out yields no exception")
    {
        std::vector<int> no_data;

        REQUIRE_NOTHROW(gdsb::out("no_data", std::begin(no_data), std::end(no_data)));
    }

    SECTION("Data of 31 is printed to stream.")
    {
        std::vector<int> data{ 1 };

        std::stringstream ss;
        gdsb::out("data", std::begin(data), std::end(data), ss);

        CHECK(ss.str() == "data: [1]\n");
    }

    SECTION("Data of 3 is printed to stream.")
    {
        std::vector<int> data{ 1, 2, 3 };

        std::stringstream ss;
        gdsb::out("data", std::begin(data), std::end(data), ss);

        CHECK(ss.str() == "data: [1, 2, 3]\n");
    }
}
