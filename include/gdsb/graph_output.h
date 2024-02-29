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

struct alignas(8) BinaryGraphHeaderIdentifier
{
    char identifier[4] = { 'G', 'D', 'S', 'B' };
    uint32_t version = 1;
};

struct alignas(8) BinaryGraphHeaderMetaDataV1
{
    uint64_t vertex_count = 2;
    uint64_t edge_count = 1;
    bool directed = false;
    bool weighted = false;
    bool dynamic = false;
};

template <typename GraphParameters = GraphParameters<FileType::binary>>
void write_header(std::ofstream& output_file, BinaryGraphHeaderIdentifier&& header_id, uint64_t const vertex_count, uint64_t const edge_count)
{
    if constexpr (GraphParameters::filetype() == FileType::binary)
    {
        if (header_id.version == 1)
        {
            char* const header_id_byte_array = reinterpret_cast<char*>(&header_id);
            output_file.write(header_id_byte_array, sizeof(decltype(header_id)));

            BinaryGraphHeaderMetaDataV1 header_data;
            header_data.vertex_count = vertex_count;
            header_data.edge_count = edge_count;
            header_data.directed = GraphParameters::is_directed();
            header_data.weighted = GraphParameters::is_weighted();
            header_data.dynamic = GraphParameters::is_dynamic();

            char* const header_data_byte_array = reinterpret_cast<char*>(&header_data);
            output_file.write(header_data_byte_array, sizeof(decltype(header_data)));
        }
    }
    else
    {
        throw std::exception("File header not defined for desired filetype.");
    }
}

template <typename GraphParameters = GraphParameters<FileType::binary>, typename Edges, typename WriteEdgeF>
void write_graph(std::ofstream& output_file, Edges&& edges, uint64_t const vertex_count, uint64_t const edge_count, WriteEdgeF&& write_edge_f)
{
    write_header<GraphParameters>(output_file, BinaryGraphHeaderIdentifier{}, vertex_count, edge_count);

    for (auto const e : edges)
    {
        write_edge_f(output_file, e);
    }

    // flush file
    output_file << std::endl;
}

} // namespace gdsb
