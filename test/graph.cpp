#include <catch2/catch_test_macros.hpp>

#include <gdsb/graph.h>

using namespace gdsb;

TEST_CASE("Graph")
{
    SECTION("Invalid Edge")
    {
        Edge invalid = invalid_edge();
        CHECK(invalid.source == std::numeric_limits<Vertex>::max());
        CHECK(invalid.target.vertex == std::numeric_limits<Vertex>::max());
        CHECK(invalid.target.weight == std::numeric_limits<Weight>::infinity());
    }

    SECTION("Gilbert Graph")
    {
        std::mt19937 engine{ 42 };

        Edges edges = gilbert_edges(0.01, 100, engine);
        CHECK(edges.size() >= 100);
    }
}
