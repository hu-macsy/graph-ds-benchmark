#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph.h>
#include <gdsb/graph_input.h>
#include <gdsb/graph_io_parameters.h>
#include <gdsb/graph_output.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

TEST_CASE("open_binary_file")
{
    std::filesystem::path file_path{ graph_path + "write_test.gdsb" };
    std::ofstream out_file = gdsb::open_binary_file(file_path);
    CHECK(out_file.is_open());
    CHECK(out_file.good());

    // Delete the file!
    REQUIRE(std::remove(file_path.c_str()) == 0);
}

TEST_CASE("write_graph, enzymes, binary")
{
    // First we read in a test graph
    gdsb::WeightedEdges32 edges;
    auto emplace = [&](gdsb::Vertex32 u, gdsb::Vertex32 v, gdsb::Weight w) {
        edges.push_back(gdsb::WeightedEdge32{ u, gdsb::Target32{ v, w } });
    };
    std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);
    auto const [vertex_count, edge_count] =
        gdsb::read_graph<gdsb::Vertex32, decltype(emplace), gdsb::EdgeListDirectedUnweightedStatic>(graph_input_unweighted_directed,
                                                                                                    std::move(emplace));

    CHECK(edges.size() == 168);

    // In case of developing a new format: use this line to produce the new test graph.
    // std::filesystem::path file_path{ graph_path + "ENZYMES_g1.bin" };

    std::filesystem::path file_path{ graph_path + "test_graph.bin" };

    std::ofstream out_file = gdsb::open_binary_file(file_path);

    REQUIRE(out_file);

    // Write Graph
    gdsb::write_graph<gdsb::BinaryDirectedUnweightedStatic, gdsb::Vertex32, gdsb::Weight>(
        out_file, edges, vertex_count, edge_count,
        [](std::ofstream& o, auto edge)
        {
            o.write(reinterpret_cast<const char*>(&edge.source), sizeof(edge.source));
            o.write(reinterpret_cast<const char*>(&edge.target.vertex), sizeof(edge.target.vertex));
        });

    // In case of developing a new format: comment this line
    REQUIRE(std::remove(file_path.c_str()) == 0);
}

TEST_CASE("write_graph, small weighted temporal, binary")
{
    // First we read in a test graph
    gdsb::TimestampedEdges32 timestamped_edges;
    auto emplace = [&](gdsb::Vertex32 u, gdsb::Vertex32 v, gdsb::Weight w, gdsb::Timestamp32 t) {
        timestamped_edges.push_back({ { u, { v, w } }, t });
    };

    std::ifstream graph_input_small_temporal(graph_path + small_weighted_temporal_graph);
    auto const [vertex_count, edge_count] =
        gdsb::read_graph<gdsb::Vertex32, decltype(emplace), gdsb::EdgeListDirectedWeightedDynamic>(graph_input_small_temporal,
                                                                                                   std::move(emplace));

    CHECK(timestamped_edges.size() == 7);

    // In case of developing a new format: use this line to produce the new test graph.
    // std::filesystem::path file_path{ graph_path + "small_graph_temporal.bin" };

    std::filesystem::path file_path{ graph_path + "small_graph_temporal_test_graph.bin" };
    std::ofstream out_file = gdsb::open_binary_file(file_path);

    REQUIRE(out_file);

    // Write Graph
    gdsb::write_graph<gdsb::BinaryDirectedWeightedDynamic, gdsb::Vertex32, gdsb::Weight>(
        out_file, timestamped_edges, vertex_count, edge_count,
        [](std::ofstream& o, auto edge)
        {
            o.write(reinterpret_cast<const char*>(&edge.edge.source), sizeof(int32_t));
            o.write(reinterpret_cast<const char*>(&edge.edge.target.vertex), sizeof(int32_t));
            o.write(reinterpret_cast<const char*>(&edge.edge.target.weight), sizeof(float));
            o.write(reinterpret_cast<const char*>(&edge.timestamp), sizeof(int32_t));
        });

    // In case of developing a new format: comment this line
    REQUIRE(std::remove(file_path.c_str()) == 0);
}