#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph.h>
#include <gdsb/graph_input.h>
#include <gdsb/graph_io_parameters.h>
#include <gdsb/graph_output.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

using namespace gdsb;

// Set this if you want to develop a new binary file format!
bool constexpr developing_new_file_format = false;

TEST_CASE("open_binary_file")
{
    std::filesystem::path file_path{ graph_path + "write_test.gdsb" };
    std::ofstream out_file = open_binary_file(file_path);
    CHECK(out_file.is_open());
    CHECK(out_file.good());

    // Delete the file!
    REQUIRE(std::remove(file_path.c_str()) == 0);
}

TEST_CASE("write_graph, enzymes, binary")
{
    // First we read in a test graph
    Edges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v) { edges.push_back(Edge32{ u, v }); };
    std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);
    auto const [vertex_count, edge_count] =
        read_graph<Vertex32, decltype(emplace), EdgeListDirectedUnweightedNoLoopStatic>(graph_input_unweighted_directed,
                                                                                        std::move(emplace));

    size_t constexpr expected_edge_count = 168u;
    CHECK(edges.size() == expected_edge_count);

    std::filesystem::path file_path = [&]() -> std::filesystem::path
    {
        if constexpr (developing_new_file_format)
        {
            return { graph_path + "ENZYMES_g1.bin" };
        }
        else
        {
            return { graph_path + "test_graph.bin" };
        }
    }();

    std::ofstream out_file = open_binary_file(file_path);

    REQUIRE(out_file);

    // Write Graph
    write_graph<BinaryDirectedUnweightedStatic, Vertex32>(out_file, edges, vertex_count, edge_count,
                                                          [](std::ofstream& o, auto edge)
                                                          {
                                                              o.write(reinterpret_cast<const char*>(&edge.source),
                                                                      sizeof(edge.source));
                                                              o.write(reinterpret_cast<const char*>(&edge.target),
                                                                      sizeof(edge.target));
                                                          });

    // Now we read in the written graph and check if we read the expected data.
    std::ifstream binary_graph(file_path);

    BinaryGraphHeader header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));
    REQUIRE(header.directed);
    REQUIRE(!header.weighted);
    REQUIRE(!header.dynamic);

    Edges32 edges_in;
    auto read_f = [&](std::ifstream& input)
    {
        edges_in.push_back(Edge32{});
        input.read((char*)&edges_in.back().source, sizeof(Vertex32));
        input.read((char*)&edges_in.back().target, sizeof(Vertex32));
        return true;
    };

    auto [vertex_count_in, edge_count_in] = read_binary_graph(binary_graph, header, std::move(read_f));

    // Note: EOF is an int (for most OS?)
    int eof_marker;
    binary_graph.read(reinterpret_cast<char*>(&eof_marker), sizeof(decltype(eof_marker)));
    REQUIRE(binary_graph.eof());

    CHECK(vertex_count_in == 38u);
    CHECK(edge_count_in == expected_edge_count);
    REQUIRE(edges_in.size() == edge_count_in);

    bool edge_25_to_2_exists = std::any_of(std::begin(edges_in), std::end(edges_in),
                                           [](Edge32 const& edge) { return edge.source == 25 && edge.target == 2; });
    CHECK(edge_25_to_2_exists);

    // In case of developing a new format: comment this line
    if constexpr (not developing_new_file_format)
    {
        REQUIRE(std::remove(file_path.c_str()) == 0);
    }
}

TEST_CASE("write_graph, aves-songbird-social, binary")
{
    // First we read in a test graph
    WeightedEdges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w) { edges.push_back(WeightedEdge32{ u, Target32{ v, w } }); };
    std::ifstream graph_input_unweighted_directed(graph_path + undirected_weighted_aves_songbird_social);

    auto const [vertex_count, edge_count] =
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedLoopStatic>(graph_input_unweighted_directed,
                                                                                      std::move(emplace));

    CHECK(vertex_count == aves_songbird_social_vertex_count);
    CHECK(edge_count == aves_songbird_social_edge_count);
    REQUIRE(edge_count == edges.size());

    std::filesystem::path file_path = [&]() -> std::filesystem::path
    {
        if constexpr (developing_new_file_format)
        {
            return { graph_path + undirected_weighted_aves_songbird_social_bin };
        }
        else
        {
            return { graph_path + "test_graph.bin" };
        }
    }();

    std::ofstream out_file = open_binary_file(file_path);

    REQUIRE(out_file);

    // Write Graph
    write_graph<BinaryUndirectedWeightedStatic, Vertex32>(
        out_file, edges, vertex_count, edge_count,
        [](std::ofstream& o, WeightedEdge32 edge)
        {
            o.write(reinterpret_cast<const char*>(&edge.source), sizeof(Vertex32));
            o.write(reinterpret_cast<const char*>(&edge.target.vertex), sizeof(Vertex32));
            o.write(reinterpret_cast<const char*>(&edge.target.weight), sizeof(Weight));
        });

    // Now we read in the written graph and check if we read the expected data.
    std::ifstream binary_graph(file_path);

    BinaryGraphHeader header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));
    REQUIRE(!header.directed);
    REQUIRE(header.weighted);
    REQUIRE(!header.dynamic);

    WeightedEdges32 edges_in;
    auto read_f = [&](std::ifstream& input)
    {
        edges_in.push_back(WeightedEdge32{});
        input.read((char*)&edges_in.back().source, sizeof(Vertex32));
        input.read((char*)&edges_in.back().target.vertex, sizeof(Vertex32));
        input.read((char*)&edges_in.back().target.weight, sizeof(Weight));
        return true;
    };

    auto [vertex_count_in, edge_count_in] = read_binary_graph(binary_graph, header, std::move(read_f));

    // Note: EOF is an int (for most OS?)
    int eof_marker;
    binary_graph.read(reinterpret_cast<char*>(&eof_marker), sizeof(decltype(eof_marker)));
    REQUIRE(binary_graph.eof());

    CHECK(vertex_count_in == aves_songbird_social_vertex_count);
    CHECK(edge_count_in == aves_songbird_social_edge_count);

    REQUIRE(edges_in.size() == aves_songbird_social_edge_count);

    // Looking for edge: {42, 81, 0.00970873786408}
    bool edge_42_to_81_exists = std::any_of(std::begin(edges_in), std::end(edges_in),
                                            [](WeightedEdge32 const& edge) {
                                                return edge.source == 42u && edge.target.vertex == 81u &&
                                                       edge.target.weight == float(0.00970873786408);
                                            });
    CHECK(edge_42_to_81_exists);

    // In case of developing a new format: comment this line
    if constexpr (not developing_new_file_format)
    {
        REQUIRE(std::remove(file_path.c_str()) == 0);
    }
}

TEST_CASE("write_graph, reptilia-tortoise-network-pv, binary")
{
    // First we read in a test graph
    TimestampedEdges32 edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Timestamp32 t)
    { edges.push_back(TimestampedEdge32{ Edge32{ u, v }, t }); };
    std::ifstream graph_input(graph_path + undirected_unweighted_temporal_reptilia_tortoise);

    auto const [vertex_count, edge_count] =
        read_graph<Vertex32, decltype(emplace), EdgeListUndirectedWeightedLoopStatic>(graph_input, std::move(emplace));

    CHECK(vertex_count == reptilia_tortoise_network_vertex_count);
    CHECK(edge_count == reptilia_tortoise_network_edge_count);
    REQUIRE(edge_count == edges.size());

    std::filesystem::path file_path = [&]() -> std::filesystem::path
    {
        if constexpr (developing_new_file_format)
        {
            return { graph_path + undirected_unweighted_temporal_reptilia_tortoise_bin };
        }
        else
        {
            return { graph_path + "test_graph.bin" };
        }
    }();

    std::ofstream out_file = open_binary_file(file_path);

    REQUIRE(out_file);

    // Write Graph
    write_graph<BinaryUndirectedUnweightedDynamic, Vertex32>(
        out_file, edges, vertex_count, edge_count,
        [](std::ofstream& o, TimestampedEdge32 edge)
        {
            o.write(reinterpret_cast<const char*>(&edge.edge.source), sizeof(Vertex32));
            o.write(reinterpret_cast<const char*>(&edge.edge.target), sizeof(Vertex32));
            o.write(reinterpret_cast<const char*>(&edge.timestamp), sizeof(Timestamp32));
        });

    // Now we read in the written graph and check if we read the expected data.
    std::ifstream binary_graph(file_path);

    BinaryGraphHeader header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));
    REQUIRE(header.timestamp_byte_size == sizeof(Timestamp32));
    REQUIRE(!header.directed);
    REQUIRE(!header.weighted);
    REQUIRE(header.dynamic);

    TimestampedEdges32 edges_in;
    auto read_f = [&](std::ifstream& input)
    {
        edges_in.push_back(TimestampedEdge32{});
        input.read((char*)&edges_in.back().edge.source, sizeof(Vertex32));
        input.read((char*)&edges_in.back().edge.target, sizeof(Vertex32));
        input.read((char*)&edges_in.back().timestamp, sizeof(Timestamp32));
        return true;
    };

    auto [vertex_count_in, edge_count_in] = read_binary_graph(binary_graph, header, std::move(read_f));

    // Note: EOF is an int (for most OS?)
    int eof_marker;
    binary_graph.read(reinterpret_cast<char*>(&eof_marker), sizeof(decltype(eof_marker)));
    REQUIRE(binary_graph.eof());

    CHECK(vertex_count_in == reptilia_tortoise_network_vertex_count);
    CHECK(edge_count_in == reptilia_tortoise_network_edge_count);

    REQUIRE(edges_in.size() == edge_count_in);

    // Looking for edge: {7, 29} {2013}
    bool edge_7_to_29_exists =
        std::any_of(std::begin(edges_in), std::end(edges_in), [](TimestampedEdge32 const& edge)
                    { return edge.edge.source == 7u && edge.edge.target == 29u && edge.timestamp == 2013u; });
    CHECK(edge_7_to_29_exists);

    // In case of developing a new format: comment this line
    if constexpr (not developing_new_file_format)
    {
        REQUIRE(std::remove(file_path.c_str()) == 0);
    }
}

TEST_CASE("write_graph, small weighted temporal, binary")
{
    // First we read in a test graph
    WeightedTimestampedEdges32 timestamped_edges;
    auto emplace = [&](Vertex32 u, Vertex32 v, Weight w, Timestamp32 t) {
        timestamped_edges.push_back({ { u, { v, w } }, t });
    };

    std::ifstream graph_input_small_temporal(graph_path + small_weighted_temporal_graph);
    auto const [vertex_count, edge_count] =
        read_graph<Vertex32, decltype(emplace), EdgeListDirectedWeightedNoLoopDynamic>(graph_input_small_temporal,
                                                                                       std::move(emplace));

    size_t constexpr expected_edge_count = 7u;
    REQUIRE(expected_edge_count == timestamped_edges.size());
    CHECK(expected_edge_count == edge_count);

    std::filesystem::path const file_path = [&]() -> std::filesystem::path
    {
        if constexpr (developing_new_file_format)
        {
            return { graph_path + small_weighted_temporal_graph_bin };
        }
        else
        {
            return { graph_path + "small_graph_temporal_test_graph.bin" };
        }
    }();

    std::ofstream out_file = open_binary_file(file_path);
    REQUIRE(out_file);

    // Write Graph
    write_graph<BinaryDirectedWeightedDynamic, Vertex32, Weight, Timestamp32>(
        out_file, timestamped_edges, vertex_count, edge_count,
        [](std::ofstream& o, auto edge)
        {
            o.write(reinterpret_cast<const char*>(&edge.edge.source), sizeof(Vertex32));
            o.write(reinterpret_cast<const char*>(&edge.edge.target.vertex), sizeof(Vertex32));
            o.write(reinterpret_cast<const char*>(&edge.edge.target.weight), sizeof(Weight));
            o.write(reinterpret_cast<const char*>(&edge.timestamp), sizeof(Timestamp32));
        });

    std::ifstream binary_graph(file_path);

    BinaryGraphHeader header = read_binary_graph_header(binary_graph);
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));
    REQUIRE(header.timestamp_byte_size == sizeof(Timestamp32));

    REQUIRE(header.directed);
    REQUIRE(header.weighted);
    REQUIRE(header.dynamic);

    WeightedTimestampedEdges32 timestamped_edges_in;
    auto read_f = [&](std::ifstream& input)
    {
        timestamped_edges_in.push_back({});

        input.read((char*)&timestamped_edges_in.back().edge.source, sizeof(Vertex32));
        input.read((char*)&timestamped_edges_in.back().edge.target.vertex, sizeof(Vertex32));
        input.read((char*)&timestamped_edges_in.back().edge.target.weight, sizeof(Weight));
        input.read((char*)&timestamped_edges_in.back().timestamp, sizeof(Timestamp32));
        return true;
    };

    auto const [vertex_count_in, edge_count_in] = read_binary_graph(binary_graph, header, std::move(read_f));
    REQUIRE(vertex_count_in == 7);
    REQUIRE(edge_count_in == expected_edge_count);

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
    REQUIRE(timestamped_edges_in.size() == edge_count_in);
    CHECK(timestamped_edges_in[idx].edge.source == 0);
    CHECK(timestamped_edges_in[idx].edge.target.vertex == 1);
    CHECK(timestamped_edges_in[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges_in[idx].timestamp == 1);

    ++idx;
    CHECK(timestamped_edges_in[idx].edge.source == 2);
    CHECK(timestamped_edges_in[idx].edge.target.vertex == 3);
    CHECK(timestamped_edges_in[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges_in[idx].timestamp == 3);

    ++idx;
    CHECK(timestamped_edges_in[idx].edge.source == 1);
    CHECK(timestamped_edges_in[idx].edge.target.vertex == 2);
    CHECK(timestamped_edges_in[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges_in[idx].timestamp == 2);

    if constexpr (not developing_new_file_format)
    {
        REQUIRE(std::remove(file_path.c_str()) == 0);
    }
}