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

} // namespace gdsb