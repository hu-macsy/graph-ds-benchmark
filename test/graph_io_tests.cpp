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
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    read_undirected_graph<Vertex32, decltype(emplace)>(graph_input, true, std::move(emplace), []() { return 1.f; });

    // Undirected: thus original edge count 103 x 2
    CHECK(edges.size() == 103 * 2);

    // CHECK if edge {16, 17} has weight 2008
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

TEST_CASE("read_undirected_graph, set max_edge_count")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    uint64_t max_edge_count = 53;
    read_undirected_graph<Vertex32, decltype(emplace)>(
        graph_input, true, std::move(emplace), []() { return 1.f; }, max_edge_count);

    // Undirected: thus original edge count 103 x 2
    CHECK(edges.size() == 54);

    // CHECK if edge {16, 17} has weight 2008
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

TEST_CASE("read_directed_graph")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    read_directed_graph<Vertex32, decltype(emplace)>(graph_input, true, std::move(emplace), []() { return 1.f; });

    CHECK(edges.size() == 103);

    // CHECK if edge {16, 17} has weight 2008
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

TEST_CASE("read_directed_graph, set max_edge_count")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    uint64_t max_edge_count = 53;
    ;
    read_directed_graph<Vertex32, decltype(emplace)>(
        graph_input, true, std::move(emplace), []() { return 1.f; }, max_edge_count);

    CHECK(edges.size() == 53);

    // CHECK if edge {16, 17} has weight 2008
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

TEST_CASE("read_unweighted")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, Target32{ v, 1.0 } }); };

    read_graph_unweighted<Vertex32, decltype(emplace)>(graph_path + unweighted_temporal_graph, std::move(emplace));

    CHECK(edges.size() == 103); // File does not have header line, this one edge less
}

TEST_CASE("read undirected unweighted")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(Edge32{ u, Target32{ v, w } }); };
    auto weight_f = []() -> Weight { return 1.f; };

    read_undirected_graph_unweighted<Vertex32, decltype(emplace), decltype(weight_f)>(graph_path + unweighted_temporal_graph,
                                                                                      std::move(emplace), std::move(weight_f));

    CHECK(edges.size() == (103 * 2)); // File does not have header line, this one edge less
}

TEST_CASE("temporal, set max_count_edges")
{
    TimestampedEdges<Edges32, Timestamps32> timestamped_edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, float w, Timestamp32 t)
    {
        timestamped_edges.edges.push_back(Edge32{ u, Target32{ v, w } });
        timestamped_edges.timestamps.push_back(t);
    };

    auto weight_f = []() -> Weight { return 1.f; };

    std::ifstream graph_input(graph_path + unweighted_temporal_graph);

    uint64_t const max_count_edges = 53;
    read_temporal_graph<Vertex32, Timestamp32>(graph_input, false, true, std::move(emplace), std::move(weight_f), max_count_edges);

    CHECK(timestamped_edges.edges.size() == max_count_edges);
}

TEST_CASE("temporal")
{
    auto emplace = [](Edges32& edges, Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, Target32{ v, 1.0 } }); };

    TimestampedEdges<Edges32, Timestamps32> timestamped_edges =
        read_temporal_graph<Vertex32, Edges32, Timestamps32, Timestamp32, decltype(emplace)>(graph_path + unweighted_temporal_graph,
                                                                                             false, std::move(emplace));

    CHECK(timestamped_edges.edges.size() == 103); // File does not have header line, this one edge less
}

TEST_CASE("Temporal Directed Weighted Graph")
{
    auto emplace = [&](Edges32& edges, Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, { v, 1.f } }); };

    TimestampedEdges<Edges32, Timestamps32> temporal_edges =
        read_temporal_graph<Vertex32, Edges32, Timestamps32, Timestamp32, decltype(emplace)>(std::move(graph_path + weighted_temporal_graph),
                                                                                             true, std::move(emplace));

    temporal_edges = sort<Edges32, Timestamps32, Timestamp32>(temporal_edges);
    Edges32 edges = std::move(temporal_edges.edges);

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
    auto emplace = [](Edges32& edges, Vertex32 u, Vertex32 v, Weight w) {
        edges.push_back(Edge32{ u, Target32{ v, w } });
    };
    auto weight_f = []() -> Weight { return 1.f; };

    TimestampedEdges<Edges32, Timestamps32> timestamped_edges =
        read_temporal_undirected_graph<Vertex32, Edges32, Timestamps32>(graph_path + unweighted_temporal_graph, false,
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
        Subgraph<Vertex32> subgraph{ 2, 5, 0, 38 };

        Edges32 edges;

        Vertex32 n = read_subgraph<Vertex32>(
            graph_input, true, false, subgraph,
            [&](Vertex32 u, Vertex32 v, Weight w) {
                edges.push_back({ u, { v, w } });
            },
            [&]() { return 1.0f; });

        CHECK(edges.size() == 16);
        CHECK(n == 38);
    }

    SECTION("Read in weighted undirected timestamped subgraph")
    {
        std::ifstream graph_input(graph_path + weighted_temporal_graph);

        Subgraph<Vertex32> subgraph{ 0, 3, 0, 7 };

        gdsb::TimestampedEdges<Edges32, Timestamps64> timestamped_edges;

        bool const directed = false;
        bool const weighted = true;

        Vertex32 n = read_temporal_subgraph<Vertex32>(
            graph_input, directed, weighted, subgraph,
            [&](Vertex32 u, Vertex32 v, Weight w, gdsb::Timestamp32 t)
            {
                timestamped_edges.edges.push_back({ u, { v, w } });
                timestamped_edges.timestamps.push_back(t);
            },
            [&]() { return 1.0f; });

        timestamped_edges = gdsb::sort<Edges32, Timestamps64, Timestamp64>(timestamped_edges);

        Edges32 edges = std::move(timestamped_edges.edges);

        CHECK(edges.size() == 6);
        CHECK(n == 7);
    }
}
