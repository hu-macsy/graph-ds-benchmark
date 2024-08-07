#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph_io.h>

#include <sstream>
#include <string>

using namespace gdsb;

TEST_CASE("read_ulong")
{
    unsigned long const a = 40;
    unsigned long const b = 200;
    unsigned long const c = 11;
    unsigned long const d = 29848572984;

    std::stringstream ss;
    ss << a << " " << b << " " << c << " " << d << "\n";
    std::string const test_string = ss.str();

    char const* string_source = test_string.c_str();
    char* string_position = nullptr;

    unsigned long const a_read = read_ulong(string_source, &string_position);
    string_source = string_position;
    unsigned long const b_read = read_ulong(string_source, &string_position);
    string_source = string_position;
    unsigned long const c_read = read_ulong(string_source, &string_position);
    string_source = string_position;
    unsigned long const d_read = read_ulong(string_source, &string_position);

    CHECK(a == a_read);
    CHECK(b == b_read);
    CHECK(c == c_read);
    CHECK(d == d_read);
}

TEST_CASE("read_graph, edge_list")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };
    std::ifstream undirected_unweighted_temporal(graph_path + undirected_unweighted_temporal_reptilia_tortoise);
    std::ifstream graph_input_weighted_temporal(graph_path + small_weighted_temporal_graph);
    std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);

    SECTION("undirected, unweighted")
    {
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedStatic>(undirected_unweighted_temporal,
                                                                                        std::move(emplace));

        // directed: thus original edge count 103
        CHECK(104 * 2 == edges.size());
        CHECK(104 * 2 == edge_count);

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (Edge32 e : edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 1.f);
                }
            }
        }
    }

    SECTION("directed, unweighted")
    {
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedStatic>(undirected_unweighted_temporal,
                                                                                      std::move(emplace));

        // directed: thus original edge count 103
        CHECK(104 == edges.size());
        CHECK(104 == edge_count);

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (Edge32 e : edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 1.f);
                }
            }
        }
    }


    SECTION("undirected, weighted")
    {
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedStatic>(undirected_unweighted_temporal,
                                                                                  std::move(emplace));

        // directed: thus original edge count 103
        CHECK(104 * 2 == edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (Edge32 e : edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 2008.f);
                }
            }
        }
    }

    SECTION("directed, weighted")
    {
        read_graph<Vertex32, decltype(emplace), EdgeListDirectedWeightedStatic>(undirected_unweighted_temporal, std::move(emplace));

        // directed: thus original edge count 103
        CHECK(104 == edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (Edge32 e : edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 2008.f);
                }
            }
        }
    }

    SECTION("directed, weighted, max_vertex set")
    {
        uint64_t const max_vertex_count = 53;
        read_graph<Vertex32, decltype(emplace), EdgeListDirectedWeightedStatic>(undirected_unweighted_temporal,
                                                                                std::move(emplace), max_vertex_count);

        // directed: thus original edge count 103
        CHECK(max_vertex_count == edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (Edge32 e : edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 2008.f);
                }
            }
        }
    }


    SECTION("undirected, unweighted, dynamic")
    {
        TimestampedEdges<Edges32, Timestamps32> timestamped_edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, float w, Timestamp32 t)
        {
            timestamped_edges.edges.push_back(Edge32{ u, Target32{ v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedDynamic, Timestamp32>(undirected_unweighted_temporal,
                                                                                                  std::move(emplace));

        CHECK(timestamped_edges.edges.size() == 104 * 2);
    }

    SECTION("undirected, weighted, dynamic")
    {
        TimestampedEdges<Edges32, Timestamps32> timestamped_edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, float w, Timestamp32 t)
        {
            timestamped_edges.edges.push_back(Edge32{ u, Target32{ v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedDynamic>(undirected_unweighted_temporal,
                                                                                   std::move(emplace));

        CHECK(timestamped_edges.edges.size() == 104 * 2);

        for (Edge32 e : timestamped_edges.edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 2008.f);
                }
            }
        }
    }

    SECTION("directed, weighted, dynamic")
    {
        TimestampedEdges<Edges32, Timestamps32> timestamped_edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, float w, Timestamp32 t)
        {
            timestamped_edges.edges.push_back(Edge32{ u, Target32{ v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        read_graph<Vertex32, decltype(emplace), EdgeListDirectedWeightedDynamic, Timestamp32>(graph_input_weighted_temporal,
                                                                                              std::move(emplace));

        timestamped_edges = sort<Edges32, Timestamps32, Timestamp32>(timestamped_edges);
        Edges32 edges = std::move(timestamped_edges.edges);

        SECTION("Read In")
        {
            REQUIRE(edges.size() == 6);
            REQUIRE(timestamped_edges.timestamps.size() == 6);
        }

        SECTION("Sort")
        {
            CHECK(edges[0].source == 0);
            CHECK(edges[1].source == 1);
            CHECK(edges[2].source == 2);
            CHECK(edges[3].source == 3);
            CHECK(edges[4].source == 3);
            CHECK(edges[5].source == 3);
        }
    }

    SECTION("read graph from string path")
    {
        std::string const input_graph_str{ graph_path + undirected_unweighted_temporal_reptilia_tortoise };
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedStatic>(input_graph_str, std::move(emplace));

        // directed: thus original edge count 103
        CHECK(104 * 2 == edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (Edge32 e : edges)
        {
            if (e.source == 16)
            {
                if (e.target.vertex == 17)
                {
                    CHECK(e.target.weight == 1.f);
                }
            }
        }
    }

    SECTION("Read in directed unweighted subgraph")
    {
        // This will read in the subgraph including vertex 1 and 2 and all it's
        // edges
        Subgraph<Vertex32> subgraph{ 2, 5, 0, 38 };
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedStatic, uint64_t, true>(
                graph_input_unweighted_directed, std::move(emplace), std::numeric_limits<uint64_t>::max(), std::move(subgraph));

        CHECK(edges.size() == 16);
        CHECK(edges.size() == edge_count);
        CHECK(vertex_count == 38);
    }

    SECTION("Read in undirected weighted dynamic subgraph")
    {
        Subgraph<Vertex32> subgraph{ 0, 3, 0, 7 };

        gdsb::TimestampedEdges<Edges32, Timestamps32> timestamped_edges;

        auto emplace_timestamped = [&](Vertex32 u, Vertex32 v, Weight w, gdsb::Timestamp32 t)
        {
            timestamped_edges.edges.push_back({ u, { v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace_timestamped), EdgeListUndirectedWeightedDynamic, Timestamp32, true>(
                graph_input_weighted_temporal, std::move(emplace_timestamped), std::numeric_limits<uint64_t>::max(),
                std::move(subgraph));

        timestamped_edges = gdsb::sort<Edges32, Timestamps32, Timestamp32>(timestamped_edges);

        Edges32 resulting_edges = std::move(timestamped_edges.edges);

        CHECK(resulting_edges.size() == 6);
        CHECK(6 == edge_count);
        CHECK(vertex_count == 7);
    }
}

TEST_CASE("read_graph, market_matrix")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };
    std::ifstream undirected_unweighted_graph(graph_path + undirected_unweighted_soc_dolphins);

    SECTION("undirected, unweighted")
    {
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), MatrixMarketUndirectedUnweightedStatic>(undirected_unweighted_graph,
                                                                                            std::move(emplace));

        // undirected
        CHECK(159 * 2 == edge_count);
        CHECK(159 * 2 == edges.size());
        // + 1 since we are counting 0 as the first vertex ID
        CHECK(62 + 1 == vertex_count);

        // CHECK if edge {43, 1} has edge weight 1.f
        for (Edge32 e : edges)
        {
            if (e.source == 43)
            {
                if (e.target.vertex == 1)
                {
                    CHECK(e.target.weight == 1.f);
                }
            }
        }
    }
}