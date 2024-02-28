#pragma once

//! This file contains functionality to write a graph to disk. This
//! functionality is specifically used to process graphs of different formats,
//! converting them to a common binary format.

#include <filesystem>
#include <fstream>

namespace gdsb
{

std::ofstream open_binary_file(std::filesystem::path const& file_path)
{
    std::ofstream output_file;
    output_file.open(file_path.c_str(), std::ios::out | std::ios::binary);
    return output_file;
}

template <typename Edges, typename WriteEdgeF>
void write_graph(std::ofstream& output_file, Edges&& edges, WriteEdgeF&& write_edge_f)
{
    for (auto const e : edges)
    {
        write_edge_f(output_file, e);
    }

    // flush file
    output_file << std::endl;
}

} // namespace gdsb
