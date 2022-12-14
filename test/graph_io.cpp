#include <catch2/catch_test_macros.hpp>

#include <gdsb/graph_io.h>

#include <string>

// https://networkrepository.com/bio-celegans.php
static std::string const graph_path =
#ifdef GDSB_TEST_GRAPH_DIR
    std::string(GDSB_TEST_GRAPH_DIR) + "/";
#else
    "test/graphs/";
#endif

static std::string unweighted_temporal_graph{ "reptilia-tortoise-network-pv.edges" };
static std::string weighted_temporal_graph{ "small_graph_temporal.edges" };

using namespace gdsb;


TEST_CASE("read_undirected_graph")
{
    Edges edges;
    auto emplace = [&](Vertex u, Vertex v, Weight w) { edges.push_back(Edge{ u, Target{ v, w } }); };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    read_undirected_graph<Vertex, decltype(emplace)>(graph_input, true, std::move(emplace), []() { return 1.f; });

    // Undirected: thus original edge count 103 x 2
    CHECK(edges.size() == 103 * 2);

    // CHECK if edge {16, 17} has weight 2008
    for (Edge e : edges)
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

TEST_CASE("read_unweighted")
{
    Edges edges;
    auto emplace = [&](Vertex u, Vertex v) { edges.push_back(Edge{ u, Target{ v, 1.0 } }); };

    read_graph_unweighted<Vertex, decltype(emplace)>(graph_path + unweighted_temporal_graph, std::move(emplace));

    CHECK(edges.size() == 103); // File does not have header line, this one edge less
}

TEST_CASE("read undirected unweighted")
{
    Edges edges;
    auto emplace = [&](Vertex u, Vertex v, Weight w) { edges.push_back(Edge{ u, Target{ v, w } }); };
    auto weight_f = []() -> Weight { return 1.f; };

    read_undirected_graph_unweighted<Vertex, decltype(emplace), decltype(weight_f)>(graph_path + unweighted_temporal_graph,
                                                                                    std::move(emplace), std::move(weight_f));

    CHECK(edges.size() == (103 * 2)); // File does not have header line, this one edge less
}

TEST_CASE("temporal")
{
    auto emplace = [](Edges& edges, Vertex u, Vertex v) { edges.push_back(Edge{ u, Target{ v, 1.0 } }); };

    TimestampedEdges<Edges> timestamped_edges =
        read_temporal_graph<Vertex, Edges, decltype(emplace)>(graph_path + unweighted_temporal_graph, false, std::move(emplace));

    CHECK(timestamped_edges.edges.size() == 103); // File does not have header line, this one edge less
}

TEST_CASE("Temporal Directed Weighted Graph")
{
    auto emplace = [&](gdsb::Edges& edges, gdsb::Vertex u, gdsb::Vertex v) {
        edges.push_back(gdsb::Edge{ u, { v, 1.f } });
    };

    gdsb::TimestampedEdges<gdsb::Edges> temporal_edges =
        gdsb::read_temporal_graph<gdsb::Vertex, gdsb::Edges, decltype(emplace)>(std::move(graph_path + weighted_temporal_graph),
                                                                                true, std::move(emplace));

    temporal_edges = gdsb::sort(temporal_edges);
    gdsb::Edges edges = std::move(temporal_edges.edges);

    SECTION("Read In")
    {
        REQUIRE(edges.size() == 6);
        REQUIRE(temporal_edges.timestamps.size() == 6);
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

TEST_CASE("temporal undirected")
{
    auto emplace = [](Edges& edges, Vertex u, Vertex v, Weight w) { edges.push_back(Edge{ u, Target{ v, w } }); };
    auto weight_f = []() -> Weight { return 1.f; };

    TimestampedEdges<Edges> timestamped_edges =
        read_temporal_undirected_graph<Vertex, Edges>(graph_path + unweighted_temporal_graph, false, std::move(emplace),
                                                      std::move(weight_f));

    CHECK(timestamped_edges.edges.size() == 103 * 2); // File does not have header line, this one edge less
}
