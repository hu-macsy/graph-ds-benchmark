#pragma once

//! This file contains functionality to write a graph to disk. This
//! functionality is specifically used to process graphs of different formats,
//! converting them to a common binary format.

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace gdsb
{

std::ofstream open_binary_file(std::filesystem::path const& file_path)
{
    std::ofstream output_file;
    output_file.open(file_path.c_str(), std::ios::out | std::ios::binary);
    return output_file;
}

template <typename GraphParameters = GraphParameters<FileType::binary>, typename VertexT, typename WeightT, typename TimestampT>
void write_header(std::ofstream& output_file, BinaryGraphHeaderIdentifier&& header_id, uint64_t const vertex_count, uint64_t const edge_count)
{
    if constexpr (GraphParameters::filetype() == FileType::binary)
    {
        if (header_id.version == binary_graph_header_version)
        {
            char* const header_id_byte_array = reinterpret_cast<char*>(&header_id);
            output_file.write(header_id_byte_array, sizeof(decltype(header_id)));

            BinaryGraphHeaderMetaDataV3 header_data;
            header_data.vertex_count = vertex_count;
            header_data.edge_count = edge_count;

            header_data.vertex_id_byte_size = sizeof(VertexT);
            header_data.weight_byte_size = sizeof(WeightT);
            header_data.timestamp_byte_size = sizeof(TimestampT);

            header_data.directed = GraphParameters::is_directed();
            header_data.weighted = GraphParameters::is_weighted();
            header_data.dynamic = GraphParameters::is_dynamic();

            char* const header_data_byte_array = reinterpret_cast<char*>(&header_data);
            output_file.write(header_data_byte_array, sizeof(BinaryGraphHeaderMetaDataV3));
        }
    }
    else
    {
        throw std::logic_error("File is not of binary type.");
    }
}

template <typename GraphParameters = GraphParameters<FileType::binary>, typename VertexT = Vertex32, typename WeightT = Weight, typename TimestampT = Timestamp32, typename Edges, typename WriteEdgeF>
void write_graph(std::ofstream& output_file, Edges&& edges, uint64_t const vertex_count, uint64_t const edge_count, WriteEdgeF&& write_edge_f)
{
    write_header<GraphParameters, VertexT, WeightT, TimestampT>(output_file, BinaryGraphHeaderIdentifier{}, vertex_count, edge_count);

    for (auto const e : edges)
    {
        write_edge_f(output_file, e);
    }

    // flush file
    output_file << std::endl;
}

} // namespace gdsb
