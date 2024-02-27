#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph_input.h>
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

TEST_CASE("write_graph")
{
    // First we read in a test graph
    gdsb::Edges32 edges;
    auto emplace = [&](gdsb::Vertex32 u, gdsb::Vertex32 v, gdsb::Weight w) {
        edges.push_back(gdsb::Edge32{ u, gdsb::Target32{ v, w } });
    };
    std::ifstream graph_input_unweighted_directed(graph_path + unweighted_directed_graph_enzymes);
    auto const [vertex_count, edge_count] =
        gdsb::read_graph<gdsb::Vertex32, decltype(emplace), gdsb::EdgeListDirectedUnweightedStatic>(graph_input_unweighted_directed,
                                                                                                    std::move(emplace));


    // Open File
    std::filesystem::path file_path{ graph_path + "write_test.gdsb" };
    std::ofstream out_file = gdsb::open_binary_file(file_path);

    REQUIRE(out_file);

    // Write Graph
    write_graph(out_file, edges,
                [](std::ofstream& o, auto edge)
                {
                    o.write(reinterpret_cast<const char*>(&edge.source), sizeof(edge.source));
                    o.write(reinterpret_cast<const char*>(&edge.target.vertex), sizeof(edge.target.vertex));
                    o.write(reinterpret_cast<const char*>(&edge.target.weight), sizeof(edge.target.weight));
                });

    // REQUIRE(std::remove(file_path.c_str()) == 0);
}