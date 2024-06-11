#include <gdsb/mpi_graph_io.h>

#include <cstddef>     // for: offsetof()
#include <type_traits> // for: is

namespace gdsb
{

namespace mpi
{

//! More information on registering MPI data types:
//! - https://stackoverflow.com/questions/33618937/trouble-understanding-mpi-type-create-struct
//! - https://docs.open-mpi.org/en/v5.0.x/man-openmpi/man3/MPI_Type_create_struct.3.html

namespace register_type
{
{
    constexpr int blocks_count = 4;
    int array_of_block_length[blocks_count] = { 1, 1, 1, 1 };

    if constexpr (!std::is_standard_layout<TimestampedEdge32>())
    {
        throw std::logic_error(
            "Timestamp32 data structure is not of standard layout and can not be commited as an MPI data type.");
    }

    constexpr std::size_t offset_source = offsetof(typename gdsb::TimestampedEdge32, edge.source);
    constexpr std::size_t offset_target_vertex = offsetof(typename gdsb::TimestampedEdge32, edge.target.vertex);
    constexpr std::size_t offset_target_weight = offsetof(typename gdsb::TimestampedEdge32, edge.target.weight);
    constexpr std::size_t offset_timestamp = offsetof(typename gdsb::TimestampedEdge32, timestamp);

    MPI_Aint array_of_displacements[blocks_count] = { static_cast<MPI_Aint>(offset_source),
                                                      static_cast<MPI_Aint>(offset_target_vertex),
                                                      static_cast<MPI_Aint>(offset_target_weight),
                                                      static_cast<MPI_Aint>(offset_timestamp) };

    MPI_Datatype array_of_types[blocks_count] = { MPI_INT32_T, MPI_INT32_T, MPI_FLOAT, MPI_INT32_T };
    MPI_Datatype mpi_timestampededges32;

    int const error = MPI_Type_create_struct(blocks_count, array_of_block_length, array_of_displacements,
                                             array_of_types, &mpi_timestampededges32);

    handle_type_create_struct_error(error);

    MPI_Type_commit(&mpi_timestampededges32);

    return mpi_timestampededges32;
}

} // namespace register_type

} // namespace mpi
} // namespace gdsb