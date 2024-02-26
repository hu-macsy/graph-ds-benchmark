#include <gdsb/graph_io.h>

#include <mpi.h>

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

template <typename T> Vertex64 read_graph_mpi(std::string const& file_name)
{
    MPI_File f = open_file(file_name);

    return 0;
}

} // namespace mpi
} // namespace gdsb