#include <gdsb/graph_io.h>

#include <cstdlib>
#include <limits>
#include <stdexcept>

#if !defined(__clang__)
#include <cerrno>
#endif

namespace gdsb
{

unsigned long read_ulong(char const* source, char** end)
{
    constexpr int numerical_base = 10;
    unsigned long const value = std::strtoul(source, end, numerical_base);

#if !defined(__clang__)
    if (errno == ERANGE)
    {
        return std::numeric_limits<unsigned long>::max();
    }
#endif

    return value;
}

} // namespace gdsb