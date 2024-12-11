#include <gdsb/mpi_graph_io.h>

#include <cstddef>     // for: offsetof()
#include <type_traits> // for: is

namespace gdsb
{

namespace mpi
{

FileWrapper::FileWrapper(std::filesystem::path const& file_path, int const mode)
{
    int error = MPI_File_open(MPI_COMM_WORLD, file_path.c_str(), mode, MPI_INFO_NULL, &m_file);

    if (error != MPI_SUCCESS)
    {
        throw std::runtime_error("Could not open file using MPI routines.");
    }
}

FileWrapper::FileWrapper(std::filesystem::path const& file_path, bool const overwrite, int const mpi_root_process, int const mode)
{
    // First we open the file requiring that the file does not exist already.
    int const test_open_error = MPI_File_open(MPI_COMM_WORLD, file_path.c_str(), mode | MPI_MODE_EXCL, MPI_INFO_NULL, &m_file);
    if (test_open_error != MPI_SUCCESS && overwrite)
    {
        // If the file does already exist, we must delete the file.
        int const rank = []()
        {
            int r = 0;
            MPI_Comm_rank(MPI_COMM_WORLD, &r);
            return r;
        }();

        if (rank == mpi_root_process)
        {
            int const error = MPI_File_delete(file_path.c_str(), MPI_INFO_NULL);
            if (error != MPI_SUCCESS)
            {
                std::string const error_msg = std::string("Could not delete file: ") + std::string(file_path.c_str());
                throw std::runtime_error(error_msg);
            }
        }

        // All MPI processes must wait for the file deletion first before
        // continuing with opening the file.
        MPI_Barrier(MPI_COMM_WORLD);

        // Now we open the file and we expect that it does not exist already. If
        // it does (again) we throw a runtime error.
        int const error = MPI_File_open(MPI_COMM_WORLD, file_path.c_str(), mode | MPI_MODE_EXCL, MPI_INFO_NULL, &m_file);
        if (error != MPI_SUCCESS)
        {
            std::string const error_msg =
                std::string("Could not open file using MPI routines: ") + std::string(file_path.c_str());
            throw std::runtime_error(error_msg);
        }
    }
}

FileWrapper::~FileWrapper() { MPI_File_close(&m_file); }

MPI_File FileWrapper::get() { return m_file; }


//! More information on registering MPI data types:
//! - https://stackoverflow.com/questions/33618937/trouble-understanding-mpi-type-create-struct
//! - https://docs.open-mpi.org/en/v5.0.x/man-openmpi/man3/MPI_Type_create_struct.3.html


MPIWeightedEdge32::MPIWeightedEdge32()
{
    constexpr int blocks_count = 3;
    constexpr int array_of_block_length[blocks_count] = { 1, 1, 1 };

    if constexpr (!std::is_standard_layout<WeightedEdge32>())
    {
        throw std::logic_error(
            "Timestamp32 data structure is not of standard layout and can not be commited as an MPI data type.");
    }

    constexpr std::size_t offset_source = offsetof(typename gdsb::WeightedEdge32, source);
    constexpr std::size_t offset_target_vertex = offsetof(typename gdsb::WeightedEdge32, target.vertex);
    constexpr std::size_t offset_target_weight = offsetof(typename gdsb::WeightedEdge32, target.weight);

    MPI_Aint const array_of_displacements[blocks_count] = { static_cast<MPI_Aint>(offset_source),
                                                            static_cast<MPI_Aint>(offset_target_vertex),
                                                            static_cast<MPI_Aint>(offset_target_weight) };

    MPI_Datatype const array_of_types[blocks_count] = { MPI_INT32_T, MPI_INT32_T, MPI_FLOAT };

    int const error = MPI_Type_create_struct(blocks_count, array_of_block_length, array_of_displacements, array_of_types, &m_type);

    handle_type_create_struct_error(error);

    MPI_Type_commit(&m_type);
}

MPI_Datatype MPIWeightedEdge32::get() const { return m_type; }

MPIEdge32::MPIEdge32()
{
    constexpr int blocks_count = 2;
    constexpr int array_of_block_length[blocks_count] = { 1, 1 };

    if constexpr (!std::is_standard_layout<Edge<Vertex32, Vertex32>>())
    {
        throw std::logic_error(
            "Timestamp32 data structure is not of standard layout and can not be commited as an MPI data type.");
    }

    constexpr std::size_t offset_source = offsetof(typename gdsb::Edge32, source);
    constexpr std::size_t offset_target = offsetof(typename gdsb::Edge32, target);

    MPI_Aint const array_of_displacements[blocks_count] = { static_cast<MPI_Aint>(offset_source),
                                                            static_cast<MPI_Aint>(offset_target) };

    MPI_Datatype const array_of_types[blocks_count] = { MPI_INT32_T, MPI_INT32_T };

    int const error = MPI_Type_create_struct(blocks_count, array_of_block_length, array_of_displacements, array_of_types, &m_type);

    handle_type_create_struct_error(error);

    MPI_Type_commit(&m_type);
}

MPI_Datatype MPIEdge32::get() const { return m_type; }

MPIWeightedTimestampedEdge32::MPIWeightedTimestampedEdge32()
{
    constexpr int blocks_count = 4;
    int const array_of_block_length[blocks_count] = { 1, 1, 1, 1 };

    if constexpr (!std::is_standard_layout<TimestampedEdge32>())
    {
        throw std::logic_error(
            "Timestamp32 data structure is not of standard layout and can not be commited as an MPI data type.");
    }

    constexpr std::size_t offset_source = offsetof(typename gdsb::WeightedTimestampedEdge32, edge.source);
    constexpr std::size_t offset_target_vertex = offsetof(typename gdsb::WeightedTimestampedEdge32, edge.target.vertex);
    constexpr std::size_t offset_target_weight = offsetof(typename gdsb::WeightedTimestampedEdge32, edge.target.weight);
    constexpr std::size_t offset_timestamp = offsetof(typename gdsb::WeightedTimestampedEdge32, timestamp);

    MPI_Aint const array_of_displacements[blocks_count] = { static_cast<MPI_Aint>(offset_source),
                                                            static_cast<MPI_Aint>(offset_target_vertex),
                                                            static_cast<MPI_Aint>(offset_target_weight),
                                                            static_cast<MPI_Aint>(offset_timestamp) };

    MPI_Datatype const array_of_types[blocks_count] = { MPI_INT32_T, MPI_INT32_T, MPI_FLOAT, MPI_INT32_T };

    int const error = MPI_Type_create_struct(blocks_count, array_of_block_length, array_of_displacements, array_of_types, &m_type);

    handle_type_create_struct_error(error);

    MPI_Type_commit(&m_type);
}

MPI_Datatype MPIWeightedTimestampedEdge32::get() const { return m_type; }

namespace binary
{

// For every call to MPI_File_read() we pass MPI_STATUS_IGNORE since we do not
// investigate any issues using the status but the returned error codes.
bool read(MPI_File const input, gdsb::Edge32& e)
{
    int ec = MPI_File_read(input, &e.source, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.target, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    return ec == MPI_SUCCESS;
}

bool read(MPI_File const input, gdsb::WeightedEdge32& e)
{
    int ec = MPI_File_read(input, &e.source, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.target.vertex, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.target.weight, 1, MPI_FLOAT, MPI_STATUS_IGNORE);
    return ec == MPI_SUCCESS;
}

bool read(MPI_File const input, gdsb::TimestampedEdge32& e)
{
    int ec = MPI_File_read(input, &e.edge.source, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.edge.target, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.timestamp, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    return ec == MPI_SUCCESS;
}

bool read(MPI_File const input, gdsb::WeightedTimestampedEdge32& e)
{
    int ec = MPI_File_read(input, &e.edge.source, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.edge.target.vertex, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.edge.target.weight, 1, MPI_FLOAT, MPI_STATUS_IGNORE);
    if (ec != MPI_SUCCESS)
    {
        return false;
    }

    ec = MPI_File_read(input, &e.timestamp, 1, MPI_INT32_T, MPI_STATUS_IGNORE);
    return ec == MPI_SUCCESS;
}
} // namespace binary

} // namespace mpi
} // namespace gdsb