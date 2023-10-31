#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph.h>
#include <gdsb/graph_io.h>

using namespace gdsb;

TEST_CASE("Vertex Counting")
{
    SECTION("Using type Edges")
    {
        Edges32 edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

        std::ifstream graph_input(graph_path + undirected_unweighted_temporal_reptilia_tortoise);
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedStatic>(graph_input, std::move(emplace));

        CHECK(vertex_count(edges) == 36);
    }


    SECTION("Using 64bit typed Edges")
    {
        Edges64 edges;
        auto emplace = [&](Vertex64 u, Vertex64 v, Weight w) { edges.push_back(Edge64{ u, Target64{ v, w } }); };

        std::ifstream graph_input(graph_path + undirected_unweighted_temporal_reptilia_tortoise);
        read_graph<Vertex64, decltype(emplace), EdgeListUndirectedWeightedStatic>(graph_input, std::move(emplace));

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

TEST_CASE("Edge Shuffling")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

    SECTION("shuffle_edges")
    {
        std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedStatic>(graph_input_unweighted_directed,
                                                                                        std::move(emplace));

        Edges32 edges_copy = edges;

        shuffle_edges(std::begin(edges), std::end(edges));

        uint32_t equality_sequence = 0;

        auto it_e = std::begin(edges);
        auto it_e_copy = std::begin(edges_copy);
        for (; it_e != std::end(edges) && it_e_copy != std::end(edges_copy) && equality_sequence <= 1; ++it_e, ++it_e_copy)
        {
            equality_sequence +=
                uint32_t((it_e->source == it_e_copy->source) && (it_e->target.vertex == it_e_copy->target.vertex));
        }

        CHECK(equality_sequence <= 1);
    }
}
