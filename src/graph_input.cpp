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

namespace binary
{

void read(std::ifstream& input, Edge32& e)
{
    input.read(reinterpret_cast<char*>(&e.source), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.target), sizeof(Vertex32));
}

void read(std::ifstream& input, gdsb::WeightedEdge32& e)
{
    input.read(reinterpret_cast<char*>(&e.source), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.target.vertex), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.target.weight), sizeof(Weight));
}

void read(std::ifstream& input, gdsb::TimestampedEdge32& e)
{
    input.read(reinterpret_cast<char*>(&e.edge.source), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.edge.target), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.timestamp), sizeof(Timestamp32));
}

void read(std::ifstream& input, gdsb::WeightedTimestampedEdge32& e)
{
    input.read(reinterpret_cast<char*>(&e.edge.source), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.edge.target.vertex), sizeof(Vertex32));
    input.read(reinterpret_cast<char*>(&e.edge.target.weight), sizeof(Weight));
    input.read(reinterpret_cast<char*>(&e.timestamp), sizeof(Timestamp32));
}

} // namespace binary

} // namespace gdsb