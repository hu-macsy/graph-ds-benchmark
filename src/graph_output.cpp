#include <gdsb/graph_output.h>

#include <gdsb/graph.h>

#include <fstream>

namespace gdsb
{

std::ofstream open_binary_file(std::filesystem::path const& file_path)
{
    std::ofstream output_file;
    output_file.open(file_path.c_str(), std::ios::out | std::ios::binary);
    return output_file;
}

namespace binary
{

void write_edge(std::ofstream& out, Edge32 const& e)
{
    out.write(reinterpret_cast<const char*>(&e.source), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.target), sizeof(Vertex32));
}

void write_edge(std::ofstream& out, WeightedEdge32 const& e)
{
    out.write(reinterpret_cast<const char*>(&e.source), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.target.vertex), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.target.weight), sizeof(Weight));
}

void write_edge(std::ofstream& out, TimestampedEdge32 const& e)
{
    out.write(reinterpret_cast<const char*>(&e.edge.source), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.edge.target), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.timestamp), sizeof(Timestamp32));
}

void write(std::ofstream& out, WeightedTimestampedEdge32 const& e)
{
    out.write(reinterpret_cast<const char*>(&e.edge.source), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.edge.target.vertex), sizeof(Vertex32));
    out.write(reinterpret_cast<const char*>(&e.edge.target.weight), sizeof(Weight));
    out.write(reinterpret_cast<const char*>(&e.timestamp), sizeof(Timestamp32));
}

} // namespace binary
} // namespace gdsb