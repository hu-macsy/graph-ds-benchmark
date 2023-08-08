#include <catch2/catch_test_macros.hpp>

// https://networkrepository.com/bio-celegans.php
static std::string const graph_path =
#ifdef GDSB_TEST_GRAPH_DIR
    std::string(GDSB_TEST_GRAPH_DIR) + "/";
#else
    "test/graphs/";
#endif

static std::string unweighted_temporal_graph{ "reptilia-tortoise-network-pv.edges" };

#include <gdsb/graph.h>
#include <gdsb/graph_io.h>

using namespace gdsb;

TEST_CASE("Vertex Counting")
{
    SECTION("Using type Edges")
    {
        Edges32 edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

        std::ifstream graph_input(graph_path + unweighted_temporal_graph);

        read_graph<Vertex32, decltype(emplace), false, true, false>(graph_input, std::move(emplace));

        CHECK(vertex_count(edges) == 36);
    }


    SECTION("Using 64bit typed Edges")
    {
        Edges64 edges;
        auto emplace = [&](Vertex64 u, Vertex64 v, Weight w) { edges.push_back(Edge64{ u, Target64{ v, w } }); };

        std::ifstream graph_input(graph_path + unweighted_temporal_graph);

        read_graph<Vertex64, decltype(emplace), false, true, false>(graph_input, std::move(emplace));

        CHECK(vertex_count(edges) == 36);
    }
}

TEST_CASE("Graph")
{
    SECTION("Invalid Edge")
    {
        Edge32 invalid = invalid_edge<Edge32>();
        CHECK(invalid.source == std::numeric_limits<Vertex32>::max());
        CHECK(invalid.target.vertex == std::numeric_limits<Vertex32>::max());
        CHECK(invalid.target.weight == std::numeric_limits<Weight>::infinity());
    }

    SECTION("Gilbert Graph")
    {
        std::mt19937 engine{ 42 };

        Edges32 edges = gilbert_edges<Vertex32, Edges32>(0.01, 100, engine);
        CHECK(edges.size() >= 100);
    }
}
