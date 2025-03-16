#pragma once

#include <gdsb/batcher.h>
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

class FileWrapper
{
public:
    FileWrapper(std::filesystem::path const&, int mode = MPI_MODE_RDONLY);
    FileWrapper(std::filesystem::path const&, bool overwrite, int mpi_root_process = 0, int mode = MPI_MODE_CREATE | MPI_MODE_WRONLY);
    ~FileWrapper();
    MPI_File get();

private:
    MPI_File m_file;
};

inline BinaryGraphHeader read_binary_graph_header(MPI_File const input)
{
    BinaryGraphHeaderIdentifier id;

    MPI_Status status;
    int error = MPI_File_read_all(input, &id, sizeof(BinaryGraphHeaderIdentifier), MPI_CHAR, &status);
    if (error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not read header identifier.");
    }

    switch (id.version)
    {
    case binary_graph_header_version:
    {
        if (!std::strcmp(id.identifier, "GDSB"))
        {
            throw std::logic_error(std::string("Binary graph file has wrong identifier: ") + std::string(id.identifier));
        }

        BinaryGraphHeader meta_data;
        error = MPI_File_read_all(input, &meta_data, sizeof(BinaryGraphHeader), MPI_CHAR, &status);
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

template <typename ReadF> bool read_binary_graph(MPI_File const input, BinaryGraphHeader const& header, ReadF&& read)
{
    bool continue_reading = true;
    for (uint64_t e = 0; e < header.edge_count && continue_reading; ++e)
    {
        continue_reading = read(input);
    }

    return continue_reading;
}

template <typename ReadF>
std::tuple<Vertex64, uint64_t> read_binary_graph_partition(MPI_File const input,
                                                           BinaryGraphHeader const& data,
                                                           ReadF&& read,
                                                           size_t const edge_size_in_bytes,
                                                           uint32_t const partition_id,
                                                           uint32_t const partition_size)
{
    uint64_t const edge_count = partition_batch_count(data.edge_count, partition_id, partition_size);

    // Header offset should be implicit since input is already read until begin of edges
    size_t const offset = batch_offset(data.edge_count, partition_id, partition_size);
    size_t const offset_in_bytes = offset * edge_size_in_bytes;

    int const error = MPI_File_seek(input, offset_in_bytes, MPI_SEEK_CUR);
    if (error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not seek to specified offset [" + std::to_string(offset) + "] within MPI file.");
    }

    bool continue_reading = true;
    for (uint64_t e = 0; e < edge_count && continue_reading; ++e)
    {
        continue_reading = read(input);
    }

    return std::make_tuple(data.vertex_count, edge_count);
}

// Parameter edges must be a pointer to the first element of the array/vector
// containing edges.
template <typename Edges>
std::tuple<Vertex64, uint64_t> all_read_binary_graph_partition(MPI_File const input,
                                                               BinaryGraphHeader const& data,
                                                               Edges* const edges,
                                                               size_t const edge_size_in_bytes,
                                                               MPI_Datatype const mpi_datatype,
                                                               uint32_t const partition_id,
                                                               uint32_t const partition_size)
{
    // Header offset should be implicit since input is already read until begin of edges
    size_t const offset = batch_offset(data.edge_count, partition_id, partition_size);
    size_t const offset_in_bytes = offset * edge_size_in_bytes;
    int const seek_error = MPI_File_seek(input, offset_in_bytes, MPI_SEEK_CUR);
    if (seek_error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not seek to specified offset [" + std::to_string(offset) + "] within MPI file.");
    }

    uint64_t const edge_count = partition_batch_count(data.edge_count, partition_id, partition_size);
    MPI_Status status;
    int const read_all_error = MPI_File_read_all(input, edges, edge_count, mpi_datatype, &status);
    if (read_all_error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not successfully read all edges from MPI file.");
    }

    return std::make_tuple(data.vertex_count, edge_count);
}

struct ReadBatch
{
    // Set this to the desired batch size.
    uint64_t batch_size{ 1 };
    // Set this to the expected size of edge in bytes.
    size_t edge_size_in_bytes{ 4 };
    // Leave this as is, will be used by all_read_binary_graph_batch().
    uint64_t count_read_in_edges{ 0 };
};

// Parameter edges must be a pointer to the first element of the array/vector
// containing edges.
//
// We do not read from file exceeding the edge count of 'data' (therefore, not
// reading past EOF). Please make sure to handle 'read_batch' correctly which is
// an in-out parameter. We set the 'read_batch.count_read_in_edges' according to
// the edge count we could read from file.
template <typename Edges>
void all_read_binary_graph_batch(MPI_File const input, BinaryGraphHeader const& data, Edges* const edges, ReadBatch& read_batch, MPI_Datatype const mpi_datatype)
{
    if (read_batch.batch_size > std::numeric_limits<int>::max())
    {
        // Have a look at the count type of MPI_File_read_all()
        throw std::runtime_error("Count of edges exceeds MPI read count type (int) maximum.");
    }

    if (read_batch.count_read_in_edges == data.edge_count)
    {
        return;
    }

    uint64_t potential_count = read_batch.batch_size;
    uint64_t potential_count_read_in_edges = read_batch.count_read_in_edges + potential_count;
    if (potential_count_read_in_edges > data.edge_count)
    {
        potential_count = data.edge_count - read_batch.count_read_in_edges;
    }

    int count = static_cast<int>(potential_count);
    read_batch.count_read_in_edges += potential_count;

    MPI_Status status;
    int const read_all_error = MPI_File_read_all(input, edges, count, mpi_datatype, &status);
    if (read_all_error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not successfully read all edges from MPI file.");
    }

    // It's not really clear from several MPI documentations I've read which
    // field actually contains the number of elements read from file given the
    // count one wants to read. The status.MPI_SOURCE field seems to
    // surprisingly provide that data point.. However, this seems to read beyond
    // EOF and still be equal to the requested count.
    //
    // Still we won't do the following since that leads to a really unclear API.
    // return status.MPI_SOURCE == count;
    //
    // TODO: Find a way to check if we read exceeding EOF. Perhaps we could
    // better model the view on the file?
}

namespace binary
{

// Returned boolean (of all read functions) indicate if anything went wrong
// during reading from input. Such read errors may also early return from the
// function. Thus, if the returned value is false, all written data to e (edge)
// may be invalid.
bool read(MPI_File const input, gdsb::Edge32& e);

bool read(MPI_File const input, gdsb::WeightedEdge32& e);

bool read(MPI_File const input, gdsb::TimestampedEdge32& e);

bool read(MPI_File const input, gdsb::WeightedTimestampedEdge32& e);

} // namespace binary

class MPIDataTypeAdapter
{
public:
    virtual ~MPIDataTypeAdapter() = default;
    virtual MPI_Datatype get() const = 0;

protected:
    MPI_Datatype m_type;
};

class MPIWeightedEdge32 : public MPIDataTypeAdapter
{
public:
    MPIWeightedEdge32();
    ~MPIWeightedEdge32() { MPI_Type_free(&m_type); }
    MPI_Datatype get() const override;
};

class MPIEdge32 : public MPIDataTypeAdapter
{
public:
    MPIEdge32();
    ~MPIEdge32() { MPI_Type_free(&m_type); }
    MPI_Datatype get() const override;
};

class MPIWeightedTimestampedEdge32 : public MPIDataTypeAdapter
{
public:
    MPIWeightedTimestampedEdge32();
    ~MPIWeightedTimestampedEdge32() { MPI_Type_free(&m_type); }
    MPI_Datatype get() const override;
};

} // namespace mpi
} // namespace gdsb