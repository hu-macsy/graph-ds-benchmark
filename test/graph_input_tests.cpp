#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph_input.h>

#include <algorithm>
#include <sstream>
#include <string>

using namespace gdsb;

TEST_CASE("read_ulong")
{
    SECTION("integers separated by spaces")
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

    SECTION("string with non integer value returns 0")
    {
        unsigned long const a = 40;
        unsigned long const b = 200;
        std::string const c = "NAS";

        std::stringstream ss;
        ss << a << " " << b << " " << c << " "
           << "\n";
        std::string const test_string = ss.str();

        char const* string_source = test_string.c_str();
        char* string_position = nullptr;

        unsigned long const a_read = read_ulong(string_source, &string_position);
        string_source = string_position;
        unsigned long const b_read = read_ulong(string_source, &string_position);
        string_source = string_position;
        unsigned long const c_read = read_ulong(string_source, &string_position);

        CHECK(a == a_read);
        CHECK(b == b_read);
        CHECK(0u == c_read);
    }
}

TEST_CASE("read_float")
{
    SECTION("floats separated by spaces")
    {
        float const a = 40.5;
        float const b = 200.3;
        float const c = 11.4;
        float const d = 0.4523;

        std::stringstream ss;
        ss << a << " " << b << " " << c << " " << d << "\n";
        std::string const test_string = ss.str();

        char const* string_source = test_string.c_str();
        char* string_position = nullptr;

        float const a_read = read_float(string_source, &string_position);
        string_source = string_position;
        float const b_read = read_float(string_source, &string_position);
        string_source = string_position;
        float const c_read = read_float(string_source, &string_position);
        string_source = string_position;
        float const d_read = read_float(string_source, &string_position);

        CHECK(a == Catch::Approx(a_read));
        CHECK(b == Catch::Approx(b_read));
        CHECK(c == Catch::Approx(c_read));
        CHECK(d == Catch::Approx(d_read));
    }

    SECTION("string with non float value returns 0")
    {
        float const a = 40.5;
        float const b = 200.3;
        std::string const c = "NAS";

        std::stringstream ss;
        ss << a << " " << b << " " << c << " "
           << "\n";
        std::string const test_string = ss.str();

        char const* string_source = test_string.c_str();
        char* string_position = nullptr;

        float const a_read = read_float(string_source, &string_position);
        string_source = string_position;
        float const b_read = read_float(string_source, &string_position);
        string_source = string_position;
        float const c_read = read_float(string_source, &string_position);

        CHECK(a == Catch::Approx(a_read));
        CHECK(b == Catch::Approx(b_read));

        CHECK(Catch::Approx(0.f) == c_read);
    }
}

TEST_CASE("read_graph, edge_list")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, v }); };

    WeightedEdges32 weighted_edges;
    auto emplace_weighted = [&](Vertex32 u, Vertex32 v, Weight w) {
        weighted_edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
    };

    std::ifstream undirected_unweighted_temporal(graph_path + undirected_unweighted_temporal_reptilia_tortoise);
    std::ifstream graph_input_weighted_temporal(graph_path + small_weighted_temporal_graph);
    std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);

    SECTION("undirected, unweighted")
    {
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedNoLoopStatic>(undirected_unweighted_temporal,
                                                                                              std::move(emplace));

        CHECK(104 * 2 == edges.size());
        CHECK(104 * 2 == edge_count);

        bool edge_16_to_17_exists = [&]()
        {
            for (Edge32 const& e : edges)
            {
                if (e.source == 16)
                {
                    if (e.target == 17)
                    {
                        return true;
                    }
                }
            }

            return false;
        }();

        CHECK(edge_16_to_17_exists);
    }

    SECTION("directed, unweighted")
    {
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedNoLoopStatic>(undirected_unweighted_temporal,
                                                                                            std::move(emplace));

        CHECK(104 == edges.size());
        CHECK(104 == edge_count);

        bool edge_16_to_17_exists = [&]()
        {
            for (Edge32 const& e : edges)
            {
                if (e.source == 16)
                {
                    if (e.target == 17)
                    {
                        return true;
                    }
                }
            }

            return false;
        }();

        CHECK(edge_16_to_17_exists);
    }

    SECTION("undirected, weighted")
    {
        read_graph<Vertex32, decltype(emplace_weighted), EdgeListUndirectedWeightedNoLoopStatic>(undirected_unweighted_temporal,
                                                                                                 std::move(emplace_weighted));

        // directed: thus original edge count 103
        CHECK(104 * 2 == weighted_edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (WeightedEdge32 e : weighted_edges)
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
        read_graph<Vertex32, decltype(emplace_weighted), EdgeListDirectedWeightedNoLoopStatic>(undirected_unweighted_temporal,
                                                                                               std::move(emplace_weighted));

        CHECK(104 == weighted_edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (WeightedEdge32 e : weighted_edges)
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
        read_graph<Vertex32, decltype(emplace_weighted), EdgeListDirectedWeightedNoLoopStatic>(undirected_unweighted_temporal,
                                                                                               std::move(emplace_weighted),
                                                                                               max_vertex_count);

        // directed: thus original edge count 103
        CHECK(max_vertex_count == weighted_edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (WeightedEdge32 e : weighted_edges)
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
        auto emplace_unweighted_dynamic = [&](Vertex32 u, Vertex32 v, Timestamp32 t)
        {
            timestamped_edges.edges.push_back(Edge32{ u, v });
            timestamped_edges.timestamps.push_back(t);
        };

        read_graph<Vertex32, decltype(emplace_unweighted_dynamic), EdgeListUndirectedUnweightedNoLoopDynamic, Timestamp32>(
            undirected_unweighted_temporal, std::move(emplace_unweighted_dynamic));

        CHECK(timestamped_edges.edges.size() == 104 * 2);
    }

    SECTION("undirected, weighted, dynamic")
    {
        TimestampedEdges<WeightedEdges32, Timestamps32> timestamped_edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, float w, Timestamp32 t)
        {
            timestamped_edges.edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedNoLoopDynamic>(undirected_unweighted_temporal,
                                                                                         std::move(emplace));

        CHECK(timestamped_edges.edges.size() == 104 * 2);

        for (WeightedEdge32 e : timestamped_edges.edges)
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
        TimestampedEdges<WeightedEdges32, Timestamps32> timestamped_edges;
        auto emplace = [&](Vertex32 u, Vertex32 v, float w, Timestamp32 t)
        {
            timestamped_edges.edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        read_graph<Vertex32, decltype(emplace), EdgeListDirectedWeightedNoLoopDynamic, Timestamp32>(graph_input_weighted_temporal,
                                                                                                    std::move(emplace));

        timestamped_edges = sort<WeightedEdges32, Timestamps32, Timestamp32>(timestamped_edges);
        WeightedEdges32 edges = std::move(timestamped_edges.edges);

        SECTION("Read In")
        {
            REQUIRE(edges.size() == 7);
            REQUIRE(timestamped_edges.timestamps.size() == 7);
        }

        SECTION("Sort")
        {
            CHECK(edges[0].source == 0);
            CHECK(edges[1].source == 1);
            CHECK(edges[2].source == 2);
            CHECK(edges[3].source == 3);
            CHECK(edges[4].source == 3);
            CHECK(edges[5].source == 3);
            CHECK(edges[6].source == 1);
        }
    }

    SECTION("read graph from string path")
    {
        std::string const input_graph_str{ graph_path + undirected_unweighted_temporal_reptilia_tortoise };
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedNoLoopStatic>(input_graph_str, std::move(emplace));

        CHECK(104 * 2 == edges.size());

        // CHECK if edge {16, 17} has weight 2008 weighted, 1 unweighted
        for (WeightedEdge32 e : weighted_edges)
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
            read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedNoLoopStatic, uint64_t, true>(
                graph_input_unweighted_directed, std::move(emplace), std::numeric_limits<uint64_t>::max(), std::move(subgraph));

        CHECK(edges.size() == 16);
        CHECK(edges.size() == edge_count);
        CHECK(vertex_count == 38);
    }

    SECTION("Read in undirected weighted dynamic subgraph")
    {
        Subgraph<Vertex32> subgraph{ 0, 3, 0, 7 };

        gdsb::TimestampedEdges<WeightedEdges32, Timestamps32> timestamped_edges;

        auto emplace_timestamped = [&](Vertex32 u, Vertex32 v, Weight w, gdsb::Timestamp32 t)
        {
            timestamped_edges.edges.push_back({ u, { v, w } });
            timestamped_edges.timestamps.push_back(t);
        };

        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace_timestamped), EdgeListUndirectedWeightedNoLoopDynamic, Timestamp32, true>(
                graph_input_weighted_temporal, std::move(emplace_timestamped), std::numeric_limits<uint64_t>::max(),
                std::move(subgraph));

        timestamped_edges = gdsb::sort<WeightedEdges32, Timestamps32, Timestamp32>(timestamped_edges);

        WeightedEdges32 resulting_edges = std::move(timestamped_edges.edges);

        CHECK(8 == resulting_edges.size());
        CHECK(8 == edge_count);
        CHECK(7 == vertex_count);
    }
}

TEST_CASE("read_graph, floating point weights")
{
    WeightedEdges32 edges;
    auto emplace = [&](Vertex32 const u, Vertex32 const v, Weight const w) {
        edges.push_back(WeightedEdge32{ u, Target32{ v, w } });
    };

    std::ifstream aves_songbird_social_input(graph_path + undirected_weighted_aves_songbird_social);
    auto const [vertex_count, edge_count] =
        gdsb::read_graph<Vertex32, decltype(emplace), gdsb::EdgeListUndirectedWeightedNoLoopStatic>(aves_songbird_social_input,
                                                                                                    std::move(emplace));

    // we start at 0 so we must have 117 + 1 vertices, the vertex count differs
    // here since all vertices must be stored starting from 0 to highest vertex
    // ID which in this case is 117
    unsigned int constexpr aves_songbird_social_vertex_count = 117 + 1;
    unsigned int constexpr aves_songbird_social_edge_count = 1027 * 2;
    Degree32 constexpr aves_songbird_social_max_degree = 56;

    SECTION("read data")
    {
        CHECK(edges.size() == aves_songbird_social_edge_count);
        CHECK(gdsb::vertex_count(edges) == aves_songbird_social_vertex_count);
    }


    SECTION("sample of edge weight is correct")
    {
        Vertex32 v_97 = 97;
        Vertex32 v_98 = 98;
        constexpr float weight_97_to_98 = 0.0028388928318f;

        auto edge = std::find_if(std::begin(edges), std::end(edges),
                                 [&](WeightedEdge32 const& e) { return e.source == v_97 && e.target.vertex == v_98; });

        REQUIRE(edge != std::end(edges));

        CHECK(weight_97_to_98 == Catch::Approx(edge->target.weight));
    }
}

TEST_CASE("read_graph, loops")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 const u, Vertex32 const v) { edges.push_back({ u, v }); };

    std::ifstream ia_southernwomen_input(graph_path + undirected_unweighted_loops_ia_southernwomen);

    // we start at 0 so we must have 18 + 1 vertices, the vertex count differs
    // here since all vertices must be stored starting from 0 to highest vertex
    // ID which in this case is 18
    unsigned int constexpr ia_southernwoman_vertex_count = 18 + 1;
    unsigned int constexpr ia_southernwoman_loop_count = 14;
    unsigned int constexpr ia_southernwoman_edge_count_loops = (89 - ia_southernwoman_loop_count) * 2 + ia_southernwoman_loop_count;
    unsigned int constexpr ia_southernwoman_edge_count_no_loops = ia_southernwoman_edge_count_loops - 14;

    SECTION("reading with loops")
    {
        auto const [vertex_count, edge_count] =
            gdsb::read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedLoopStatic>(ia_southernwomen_input,
                                                                                                  std::move(emplace));
        CHECK(vertex_count == ia_southernwoman_vertex_count);
        CHECK(edge_count == ia_southernwoman_edge_count_loops);
    }

    SECTION("reading no loops")
    {
        auto const [vertex_count, edge_count] =
            gdsb::read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedNoLoopStatic>(ia_southernwomen_input,
                                                                                                    std::move(emplace));
        CHECK(vertex_count == ia_southernwoman_vertex_count);
        CHECK(edge_count == ia_southernwoman_edge_count_no_loops);
    }
}

TEST_CASE("read_graph, market_matrix")
{
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, v }); };
    std::ifstream undirected_unweighted_graph(graph_path + undirected_unweighted_soc_dolphins);

    SECTION("undirected, unweighted")
    {
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), MatrixMarketUndirectedUnweightedNoLoopStatic>(undirected_unweighted_graph,
                                                                                                  std::move(emplace));

        // undirected
        CHECK(159 * 2 == edge_count);
        CHECK(159 * 2 == edges.size());
        // + 1 since we are counting 0 as the first vertex ID
        CHECK(62 + 1 == vertex_count);

        bool edge_43_to_1_exists = [&]()
        {
            for (Edge32 const& e : edges)
            {
                if (e.source == 43)
                {
                    if (e.target == 1)
                    {
                        return true;
                    }
                }
            }

            return false;
        }();

        CHECK(edge_43_to_1_exists);
    }
}

TEST_CASE("read_binary_graph, small weighted temporal")
{
    WeightedTimestampedEdges32 timestamped_edges;
    auto read_f = [&](std::ifstream& input)
    {
        timestamped_edges.push_back({});

        input.read((char*)&timestamped_edges.back().edge.source, sizeof(Vertex32));
        input.read((char*)&timestamped_edges.back().edge.target.vertex, sizeof(Vertex32));
        input.read((char*)&timestamped_edges.back().edge.target.weight, sizeof(Weight));
        input.read((char*)&timestamped_edges.back().timestamp, sizeof(Timestamp32));
        return true;
    };

    std::ifstream binary_graph(graph_path + small_weighted_temporal_graph_bin);

    BinaryGraphHeaderMetaDataV3 header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    auto const [vertex_count, edge_count] = read_binary_graph(binary_graph, header, std::move(read_f));
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 7);

    // Note: EOF is an int (for most OS?)
    int eof_marker;
    binary_graph.read(reinterpret_cast<char*>(&eof_marker), sizeof(decltype(eof_marker)));
    REQUIRE(binary_graph.eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 1 4 1 8
    // 3 4 1 4
    // 3 5 1 6
    // 3 6 1 7

    size_t idx = 0;
    REQUIRE(timestamped_edges.size() == edge_count);
    CHECK(timestamped_edges[idx].edge.source == 0);
    CHECK(timestamped_edges[idx].edge.target.vertex == 1);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 1);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 2);
    CHECK(timestamped_edges[idx].edge.target.vertex == 3);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 3);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 2);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 2);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 8);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 4);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 5);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 6);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 6);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 7);

    ++idx;
    REQUIRE(timestamped_edges.size() == idx);
}

TEST_CASE("read_binary_graph, undirected, unweighted, static")
{
    Edges32 edges;
    auto read_f = [&](std::ifstream& input)
    {
        edges.push_back(Edge32{});
        input.read((char*)&edges.back().source, sizeof(Vertex32));
        input.read((char*)&edges.back().target, sizeof(Vertex32));
        return true;
    };

    std::ifstream binary_graph(graph_path + unweighted_directed_graph_enzymes_bin);

    BinaryGraphHeaderMetaDataV3 header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    auto [vertex_count, edge_count] = read_binary_graph(binary_graph, header, std::move(read_f));

    // Note: EOF is an int (for most OS?)
    int eof_marker;
    binary_graph.read(reinterpret_cast<char*>(&eof_marker), sizeof(decltype(eof_marker)));
    REQUIRE(binary_graph.eof());

    CHECK(vertex_count == 38);
    CHECK(edge_count == 168);
    REQUIRE(edges.size() == edge_count);

    bool edge_25_to_2_exists = std::any_of(std::begin(edges), std::end(edges),
                                           [](Edge32 const& edge) { return edge.source == 25 && edge.target == 2; });
    CHECK(edge_25_to_2_exists);

    bool edge_source_0_does_not_exist =
        std::none_of(std::begin(edges), std::end(edges), [](Edge32 const& edge) { return edge.source == 0; });
    CHECK(edge_source_0_does_not_exist);
}

TEST_CASE("read_binary_graph_partition, small weighted temporal, partition id 0, partition size 2")
{
    WeightedTimestampedEdges32 timestamped_edges;
    auto read_f = [&](std::ifstream& input)
    {
        timestamped_edges.push_back({});

        input.read((char*)&timestamped_edges.back().edge.source, sizeof(Vertex32));
        input.read((char*)&timestamped_edges.back().edge.target.vertex, sizeof(Vertex32));
        input.read((char*)&timestamped_edges.back().edge.target.weight, sizeof(Weight));
        input.read((char*)&timestamped_edges.back().timestamp, sizeof(Timestamp32));
        return true;
    };

    std::ifstream binary_graph(graph_path + small_weighted_temporal_graph_bin);

    BinaryGraphHeaderMetaDataV3 header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    uint32_t partition_id = 0;
    uint32_t partition_size = 2;
    auto const [vertex_count, edge_count] =
        read_binary_graph_partition(binary_graph, header, std::move(read_f),
                                    sizeof(TimestampedEdge<WeightedEdge32, Timestamp32>), partition_id, partition_size);
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 3);

    CHECK(!binary_graph.eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 1 4 1 8
    // 3 4 1 4
    // 3 5 1 6
    // 3 6 1 7

    size_t idx = 0;
    REQUIRE(timestamped_edges.size() == edge_count);
    CHECK(timestamped_edges[idx].edge.source == 0);
    CHECK(timestamped_edges[idx].edge.target.vertex == 1);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 1);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 2);
    CHECK(timestamped_edges[idx].edge.target.vertex == 3);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 3);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 2);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 2);

    ++idx;
    REQUIRE(timestamped_edges.size() == idx);
}

TEST_CASE("read_binary_graph_partition, small weighted temporal, partition id 1, partition size 2")
{
    WeightedTimestampedEdges32 timestamped_edges;
    auto read_f = [&](std::ifstream& input)
    {
        timestamped_edges.push_back({});

        input.read((char*)&timestamped_edges.back().edge.source, sizeof(Vertex32));
        input.read((char*)&timestamped_edges.back().edge.target.vertex, sizeof(Vertex32));
        input.read((char*)&timestamped_edges.back().edge.target.weight, sizeof(Weight));
        input.read((char*)&timestamped_edges.back().timestamp, sizeof(Timestamp32));
        return true;
    };

    std::ifstream binary_graph(graph_path + small_weighted_temporal_graph_bin);

    BinaryGraphHeaderMetaDataV3 header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    uint32_t partition_id = 1;
    uint32_t partition_size = 2;
    auto const [vertex_count, edge_count] =
        read_binary_graph_partition(binary_graph, header, std::move(read_f),
                                    sizeof(TimestampedEdge<WeightedEdge32, Timestamp32>), partition_id, partition_size);
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 4);

    // Note: EOF is an int (for most OS?)
    int eof_marker;
    binary_graph.read(reinterpret_cast<char*>(&eof_marker), sizeof(decltype(eof_marker)));
    REQUIRE(binary_graph.eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 1 4 1 8
    // 3 4 1 4
    // 3 5 1 6
    // 3 6 1 7

    size_t idx = 0;
    REQUIRE(timestamped_edges.size() == edge_count);
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 8);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 4);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 5);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 6);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 6);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 7);

    ++idx;
    REQUIRE(timestamped_edges.size() == idx);
}

TEST_CASE("insert_return_edges")
{
    SECTION("unweighted edges, all return edges exist")
    {
        Edges32 edges;
        auto emplace = [&](Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, v }); };

        std::ifstream graph_input_unweighted_directed(graph_path + undirected_unweighted_soc_dolphins);

        // intentionally reading graph as directed
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), MatrixMarketDirectedUnweightedNoLoopStatic>(graph_input_unweighted_directed,
                                                                                                std::move(emplace));

        size_t const original_edge_size = edges.size();

        auto copy_f = [](Edge32& original, Edge32& copy)
        {
            copy.source = original.target;
            copy.target = original.source;
        };

        insert_return_edges(std::move(copy_f), edges);

        REQUIRE((original_edge_size * 2) == edges.size());

        bool all_return_edges_found = true;
        auto original_begin = std::begin(edges);
        auto original_end = std::begin(edges);
        std::advance(original_end, original_edge_size);

        for (auto it = original_begin; it != original_end && all_return_edges_found; ++it)
        {
            auto found_it =
                std::find_if(original_end, std::end(edges),
                             [&](Edge32 const& e) { return e.source == it->target && e.target == it->source; });
            all_return_edges_found = found_it != std::end(edges);
        }

        CHECK(all_return_edges_found);
    }

    SECTION("unweighted edges, no duplications")
    {
        Edges32 edges;
        auto emplace = [&](Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, v }); };

        std::ifstream graph_input_unweighted_directed(graph_path + undirected_unweighted_soc_dolphins);

        // intentionally reading graph as directed
        auto const [vertex_count, edge_count] =
            read_graph<Vertex32, decltype(emplace), MatrixMarketDirectedUnweightedNoLoopStatic>(graph_input_unweighted_directed,
                                                                                                std::move(emplace));

        REQUIRE(edges.size() == 159u);

        auto no_duplicates_found = [&]() -> bool
        {
            bool no_duplicates = true;
            for (auto e = std::begin(edges); e != std::end(edges) & no_duplicates; ++e)
            {
                auto next = std::next(e);
                auto duplicate =
                    std::find_if(next, std::end(edges),
                                 [&](Edge32 const& d) { return e->source == d.source && e->target == d.target; });

                no_duplicates = duplicate == std::end(edges);
            }
            return no_duplicates;
        };

        REQUIRE(no_duplicates_found());

        size_t const original_edge_size = edges.size();

        auto copy_f = [](Edge32& original, Edge32& copy)
        {
            copy.source = original.target;
            copy.target = original.source;
        };

        insert_return_edges(std::move(copy_f), edges);

        REQUIRE((original_edge_size * 2) == edges.size());
        CHECK(no_duplicates_found());
    }
}