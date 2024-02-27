#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/graph_input.h>
#include <gdsb/graph_output.h>

#include <filesystem>
#include <fstream>

TEST_CASE("Write file")
{
    std::filesystem::path file_path{ graph_path + "write_test.gdsb" };
    std::ofstream out_file = gdsb::open_binary_file(file_path);
    CHECK(out_file.is_open());
}