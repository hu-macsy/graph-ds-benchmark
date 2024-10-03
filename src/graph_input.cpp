#include <gdsb/graph_input.h>

#include <cstdlib>
#include <limits>
#include <stdexcept>

#if !defined(__clang__)
#include <cerrno>
#endif

namespace gdsb
{

// For both read_ulong() and read_float() we're using std::strtoul() and
// std::strtof() respectively setting errno to ERANGE in case of an error. Clang
// does not support the use of ERANGE. We therefore ignore the error check for
// Clang. Please see https://en.cppreference.com/w/cpp/string/byte/strtof for
// more information.
unsigned long read_ulong(char const* source, char** end)
{
    constexpr int numerical_base = 10;
    unsigned long const value = std::strtoul(source, end, numerical_base);

#if !defined(__clang__)
    if (errno == ERANGE)
    {
        errno = 0;
        return std::numeric_limits<unsigned long>::max();
    }
#endif

    return value;
}

float read_float(char const* source, char** end)
{
    float const value = std::strtof(source, end);

#if !defined(__clang__)
    if (errno == ERANGE)
    {
        errno = 0;
        // TODO: Print warning message if an error was detected during string
        // literal parsing.
        return std::numeric_limits<float>::infinity();
    }
#endif

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

uint64_t edge_offset(uint64_t total_edge_count, uint32_t const partition_id, uint32_t const partition_size)
{
    return total_edge_count / partition_size * partition_id;
}

} // namespace gdsb