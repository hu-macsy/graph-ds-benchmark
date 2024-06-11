#pragma once

#include <gdsb/graph_input.h>
#include <gdsb/mpi_error_handler.h>

#include <mpi.h>

#include <array>
#include <filesystem>
#include <stdexcept>

namespace gdsb
{
namespace mpi
{

inline MPI_File open_file(std::filesystem::path const& file_path)
{
    MPI_File f;
    int error = MPI_File_open(MPI_COMM_WORLD, file_path.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

    if (error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not open file using MPI routines.");
    }

    return f;
}

inline BinaryGraphHeaderMetaDataV1 read_binary_graph_header(MPI_File input)
{
    BinaryGraphHeaderIdentifier id;

    MPI_Status status;
    int error = MPI_File_read_all(input, &id, sizeof(BinaryGraphHeaderIdentifier), MPI_CHAR, &status);
    if (error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not read header identifier.");
    }

    GraphParameters<FileType::binary> graph_parameters;

    switch (id.version)
    {
    case 1:
    {
        if (!std::strcmp(id.identifier, "GDSB"))
        {
            throw std::logic_error(std::string("Binary graph file has wrong identifier: ") + std::string(id.identifier));
        }

        BinaryGraphHeaderMetaDataV1 meta_data;
        error = MPI_File_read_all(input, &meta_data, sizeof(BinaryGraphHeaderMetaDataV1), MPI_CHAR, &status);
        if (error != MPI_SUCCESS)
        {
            throw std::runtime_error("Could not read meta data.");
        }

        return meta_data;
    }
    default:
        throw std::logic_error("Binary graph version not supported: " + std::to_string(id.version));
    }
}

template <typename ReadF> std::tuple<Vertex64, uint64_t> read_binary_graph(MPI_File input, ReadF&& read)
{
    BinaryGraphHeaderMetaDataV1 const data = read_binary_graph_header(input);

    bool continue_reading = true;
    for (uint64_t e = 0; e < data.edge_count && continue_reading; ++e)
    {
        continue_reading = read(input);
    }

    return std::make_tuple(data.vertex_count, data.edge_count);
}


template <typename ReadF>
std::tuple<Vertex64, uint64_t> read_binary_graph_partition(MPI_File input,
                                                           BinaryGraphHeaderMetaDataV1 const& data,
                                                           ReadF&& read,
                                                           size_t edge_size_in_bytes,
                                                           uint32_t const partition_id,
                                                           uint32_t const partition_size)
{
    uint64_t const edge_count = partition_edge_count(data.edge_count, partition_id, partition_size);

    // Header offset should be implicit since input is already read until begin of edges
    size_t const offset = edge_offset(data.edge_count, partition_id, partition_size);

    MPI_File_seek(input, offset * edge_size_in_bytes, MPI_SEEK_CUR);

    bool continue_reading = true;
    for (uint64_t e = 0; e < edge_count && continue_reading; ++e)
    {
        continue_reading = read(input);
    }

    return std::make_tuple(data.vertex_count, edge_count);
}

template <typename Edges>
std::tuple<Vertex64, uint64_t> all_read_binary_graph_partition(MPI_File input,
                                                               BinaryGraphHeaderMetaDataV1 const& data,
                                                               Edges& edges,
                                                               size_t edge_size_in_bytes,
                                                               MPI_Datatype mpi_datatype,
                                                               uint32_t const partition_id,
                                                               uint32_t const partition_size)
{
    uint64_t const edge_count = partition_edge_count(data.edge_count, partition_id, partition_size);
    edges.resize(edge_count);

    // Header offset should be implicit since input is already read until begin of edges
    size_t const offset = edge_offset(data.edge_count, partition_id, partition_size);

    MPI_File_seek(input, offset * edge_size_in_bytes, MPI_SEEK_CUR);

    MPI_Status status;
    MPI_File_read_all(input, &(edges[0]), edge_count, mpi_datatype, &status);

    return std::make_tuple(data.vertex_count, edge_count);
}

MPI_Datatype register_timestamped_edge_32();

} // namespace mpi
} // namespace gdsb