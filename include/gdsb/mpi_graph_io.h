#include <gdsb/graph_input.h>

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

// inline get_line(MPI_File file)
// {
//     char line[1024];

//     while (fgets(line, 1024, fp))
//     {
//         printf("%s\n", line);
//     }
// }

// template <typename EmplaceF, typename GraphParameters = GraphParameters<FileType::edge_list>, typename Timestamp = uint64_t>
// read_graph_partition_mpi(MPI_File file, EmplaceF&& emplace, uint32_t const partition_id, uint32_t const partition_count, uint64_t const edge_count)
// {
//     // The issue with MPI IO is that it can't really read lines.
//     // So for now we drop using MPI IO for this purpose.
//     //
//     uint64_t partition_edge_count = edge_count / partition_count;
//     if (partition_id == partition_count - 1)
//     {
//         partition_edge_count += edge_count % partition_count;
//     }

//     size_t const edge_offset = (edge_count / partition_count) * partition_id;

//     char line[1024];

//     // edge = new T[e_num];
//     // fseek(f, f_offset, SEEK_SET);
//     // auto ret = fread(edge, sizeof(T), e_num, f);
//     // assert(ret == e_num);
//     // fclose(f);
// }

} // namespace mpi
} // namespace gdsb