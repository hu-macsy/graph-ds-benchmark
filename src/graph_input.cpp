#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace gdsb
{

unsigned long read_ulong(char const* source, char** end)
{
    constexpr int numerical_base = 10;
    unsigned long const value = std::strtoul(source, end, numerical_base);

    if (errno == ERANGE)
    {
        return std::numeric_limits<unsigned long>::max();
    }

    return value;
}

uint64_t partition_edge_count(uint64_t const total_edge_count, uint32_t const partition_id, uint32_t const partition_size)
{
    uint64_t partition_edge_count = total_edge_count / partition_size;
    if (partition_id == partition_size - 1)
    {
        partition_edge_count += total_edge_count % partition_size;
    }

    return partition_edge_count;
}

} // namespace gdsb