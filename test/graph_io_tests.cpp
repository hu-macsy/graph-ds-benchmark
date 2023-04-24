#include <catch2/catch_test_macros.hpp>

#include <gdsb/graph_io.h>

#include <sstream>
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
static std::string unweighted_directed_graph{ "ENZYMES_g1.edges" };

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

TEST_CASE("read_directed_graph")
{
    Edges edges;
    auto emplace = [&](Vertex u, Vertex v, Weight w) { edges.push_back(Edge{ u, Target{ v, w } }); };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    read_directed_graph<Vertex, decltype(emplace)>(graph_input, true, std::move(emplace), []() { return 1.f; });

    CHECK(edges.size() == 103);

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

    TimestampedEdges<Edges, Timestamps> timestamped_edges =
        read_temporal_graph<Vertex, Edges, Timestamps, Timestamp, decltype(emplace)>(graph_path + unweighted_temporal_graph,
                                                                                     false, std::move(emplace));

    CHECK(timestamped_edges.edges.size() == 103); // File does not have header line, this one edge less
}

TEST_CASE("Temporal Directed Weighted Graph")
{
    auto emplace = [&](Edges& edges, Vertex u, Vertex v) { edges.push_back(Edge{ u, { v, 1.f } }); };

    TimestampedEdges<Edges, Timestamps> temporal_edges =
        read_temporal_graph<Vertex, Edges, Timestamps, Timestamp, decltype(emplace)>(std::move(graph_path + weighted_temporal_graph),
                                                                                     true, std::move(emplace));

    temporal_edges = sort<Edges, Timestamps, Timestamp>(temporal_edges);
    Edges edges = std::move(temporal_edges.edges);

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

    TimestampedEdges<Edges, Timestamps> timestamped_edges =
        read_temporal_undirected_graph<Vertex, Edges, Timestamps>(graph_path + unweighted_temporal_graph, false,
                                                                  std::move(emplace), std::move(weight_f));

    CHECK(timestamped_edges.edges.size() == 103 * 2); // File does not have header line, this one edge less
}

TEST_CASE("Decomposition")
{
    using namespace gdsb;

    SECTION("Read in unweighted undirected subgraph")
    {
        std::ifstream graph_input(graph_path + unweighted_directed_graph);

        // This will read in the subgraph including vertex 1 and 2 and all it's
        // edges
        Subgraph<Vertex> subgraph{ 2, 5, 0, 38 };

        Edges edges;

        Vertex n = read_subgraph<Vertex>(
            graph_input, true, false, subgraph,
            [&](Vertex u, Vertex v, Weight w) {
                edges.push_back({ u, { v, w } });
            },
            [&]() { return 1.0f; });

        CHECK(edges.size() == 16);
        CHECK(n == 38);
    }

    SECTION("Read in weighted undirected timestamped subgraph")
    {
        std::ifstream graph_input(graph_path + weighted_temporal_graph);

        Subgraph<Vertex> subgraph{ 0, 3, 0, 7 };

        gdsb::TimestampedEdges<Edges, Timestamps64> timestamped_edges;

        bool const directed = false;
        bool const weighted = true;

        Vertex n = read_temporal_subgraph<Vertex>(
            graph_input, directed, weighted, subgraph,
            [&](Vertex u, Vertex v, Weight w, gdsb::Timestamp t)
            {
                timestamped_edges.edges.push_back({ u, { v, w } });
                timestamped_edges.timestamps.push_back(t);
            },
            [&]() { return 1.0f; });

        timestamped_edges = gdsb::sort<Edges, Timestamps64, Timestamp64>(timestamped_edges);

        Edges edges = std::move(timestamped_edges.edges);

        CHECK(edges.size() == 6);
        CHECK(n == 7);
    }
}
