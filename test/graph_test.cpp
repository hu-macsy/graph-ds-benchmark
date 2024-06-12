#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph.h>
#include <gdsb/graph_input.h>

using namespace gdsb;

TEST_CASE("Vertex Counting")
{
    SECTION("Using type Edges")
    {
        WeightedEdges32 edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) {
            edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
        };

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
        WeightedEdge32 invalid = invalid_edge<WeightedEdge32>();
        CHECK(invalid.source == std::numeric_limits<Vertex32>::max());
        CHECK(invalid.target.vertex == std::numeric_limits<Vertex32>::max());
        CHECK(invalid.target.weight == std::numeric_limits<Weight>::infinity());
    }

    SECTION("Gilbert Graph")
    {
        std::mt19937 engine{ 42 };

        WeightedEdges32 edges = gilbert_edges<Vertex32, WeightedEdges32>(0.01, 100, engine);
        CHECK(edges.size() >= 100);
    }
}

TEST_CASE("Edge Shuffling")
{
    SECTION("shuffle_edges, sequence not equal")
    {
        WeightedEdges32 edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) {
            edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
        };

        std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedStatic>(graph_input_unweighted_directed,
                                                                                        std::move(emplace));

        WeightedEdges32 edges_copy = edges;

        shuffle_edges(std::begin(edges), std::end(edges));

        uint32_t equality_sequence = 0;
        auto sum_edges = std::make_tuple<uint32_t, uint32_t>(0, 0);
        auto sum_edges_copy = std::make_tuple<uint32_t, uint32_t>(0, 0);

        auto it_e = std::begin(edges);
        auto it_e_copy = std::begin(edges_copy);
        for (; it_e != std::end(edges) && it_e_copy != std::end(edges_copy); ++it_e, ++it_e_copy)
        {
            Vertex32 u = it_e->source;
            Vertex32 v = it_e->target.vertex;
            Vertex32 u_c = it_e_copy->source;
            Vertex32 v_c = it_e_copy->target.vertex;

            equality_sequence += uint32_t((u == u_c) && (v == v_c));
            std::get<0>(sum_edges) += u;
            std::get<1>(sum_edges) += v;
            std::get<0>(sum_edges_copy) += u_c;
            std::get<1>(sum_edges_copy) += v_c;
        }

        REQUIRE(sum_edges == sum_edges_copy);

        //! Here we check that there is no sequence equal between the shuffled
        //! edges and the copy before shuffling. Having a maximum of (1/2 *
        //! size) of edges might seem a bit generous, but we do not have any
        //! data that shows what the maximum sequence should be that is equal
        //! for both. Thus, we chose (1/2 * size) would be sufficient for now.
        CHECK(equality_sequence < uint32_t(edges.size() / 2));
    }

    SECTION("shuffle_timestamped_edges, sequence not equal")
    {
        TimestampedEdges<WeightedEdges32, Timestamps32> timestamped_edges;
        auto emplace = [&](Timestamp32 t, Vertex32 u, Vertex32 v, Weight w)
        {
            timestamped_edges.timestamps.push_back(t);
            timestamped_edges.edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
        };

        std::ifstream graph_input_unweighted_temporal(graph_path + undirected_unweighted_temporal_reptilia_tortoise);
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedDynamic>(graph_input_unweighted_temporal,
                                                                                         std::move(emplace));

        WeightedEdges32 edges_copy = timestamped_edges.edges;

        shuffle_timestamped_edges(timestamped_edges);

        WeightedEdges32 const& edges = timestamped_edges.edges;

        uint32_t equality_sequence = 0;
        auto sum_edges = std::make_tuple<uint32_t, uint32_t>(0, 0);
        auto sum_edges_copy = std::make_tuple<uint32_t, uint32_t>(0, 0);

        auto it_e = std::begin(edges);
        auto it_e_copy = std::begin(edges_copy);
        for (; it_e != std::end(edges) && it_e_copy != std::end(edges) && equality_sequence <= 1; ++it_e, ++it_e_copy)
        {
            Vertex32 u = it_e->source;
            Vertex32 v = it_e->target.vertex;
            Vertex32 u_c = it_e_copy->source;
            Vertex32 v_c = it_e_copy->target.vertex;

            equality_sequence += uint32_t((u == u_c) && (v == v_c));
            std::get<0>(sum_edges) += u;
            std::get<1>(sum_edges) += v;
            std::get<0>(sum_edges_copy) += u_c;
            std::get<1>(sum_edges_copy) += v_c;
        }

        REQUIRE(sum_edges == sum_edges_copy);

        CHECK(equality_sequence < uint32_t(edges.size() / 2));
    }
}
