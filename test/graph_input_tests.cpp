#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph_input.h>

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
            read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedStatic>(undirected_unweighted_temporal,
                                                                                        std::move(emplace));

        // directed: thus original edge count 103
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
            read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedStatic>(undirected_unweighted_temporal,
                                                                                      std::move(emplace));

        // directed: thus original edge count 103
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
        read_graph<Vertex32, decltype(emplace_weighted), EdgeListUndirectedWeightedStatic>(undirected_unweighted_temporal,
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
        read_graph<Vertex32, decltype(emplace_weighted), EdgeListDirectedWeightedStatic>(undirected_unweighted_temporal,
                                                                                         std::move(emplace_weighted));

        // directed: thus original edge count 103
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
        read_graph<Vertex32, decltype(emplace_weighted), EdgeListDirectedWeightedStatic>(undirected_unweighted_temporal,
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

        read_graph<Vertex32, decltype(emplace_unweighted_dynamic), EdgeListUndirectedUnweightedDynamic, Timestamp32>(
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

        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedDynamic>(undirected_unweighted_temporal,
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

        read_graph<Vertex32, decltype(emplace), EdgeListDirectedWeightedDynamic, Timestamp32>(graph_input_weighted_temporal,
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
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedUnweightedStatic>(input_graph_str, std::move(emplace));

        // directed: thus original edge count 103
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
            read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedStatic, uint64_t, true>(
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
            read_graph<Vertex32, decltype(emplace_timestamped), EdgeListUndirectedWeightedDynamic, Timestamp32, true>(
                graph_input_weighted_temporal, std::move(emplace_timestamped), std::numeric_limits<uint64_t>::max(),
                std::move(subgraph));

        timestamped_edges = gdsb::sort<WeightedEdges32, Timestamps32, Timestamp32>(timestamped_edges);

        WeightedEdges32 resulting_edges = std::move(timestamped_edges.edges);

        CHECK(8 == resulting_edges.size());
        CHECK(8 == edge_count);
        CHECK(7 == vertex_count);
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
            read_graph<Vertex32, decltype(emplace), MatrixMarketUndirectedUnweightedStatic>(undirected_unweighted_graph,
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
    TimestampedEdges32 timestamped_edges;
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

    BinaryGraphHeaderMetaDataV1 header = read_binary_graph_header(binary_graph);
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

    BinaryGraphHeaderMetaDataV1 header = read_binary_graph_header(binary_graph);
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

TEST_CASE("partition_edge_count, on enzymes graph")
{
    std::ifstream binary_graph(graph_path + unweighted_directed_graph_enzymes_bin);
    BinaryGraphHeaderMetaDataV1 header = read_binary_graph_header(binary_graph);

    SECTION("partition size 2")
    {
        uint32_t partition_size = 2;

        uint32_t partition_id = 0;
        uint64_t edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 84);

        partition_id = 1;
        edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 84);
    }

    SECTION("partition size 5")
    {
        uint32_t partition_size = 5;

        uint32_t partition_id = 0;
        uint64_t edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 1;
        edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 2;
        edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 3;
        edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 4;
        edge_count = partition_edge_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 36);
    }
}

TEST_CASE("read_binary_graph_partition, small weighted temporal, partition id 0, partition size 2")
{
    TimestampedEdges32 timestamped_edges;
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

    BinaryGraphHeaderMetaDataV1 header = read_binary_graph_header(binary_graph);
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
    TimestampedEdges32 timestamped_edges;
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

    BinaryGraphHeaderMetaDataV1 header = read_binary_graph_header(binary_graph);
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