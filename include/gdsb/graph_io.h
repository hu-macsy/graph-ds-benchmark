#pragma once

#include <gdsb/graph.h>

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace gdsb
{

//! Reads in space separated integers of type unsigned long. Returns max if
//! integer at position source can not be interpreted as a number.
//! @param source   Points to the beginning of the character (sub string) to be
//! interpreted.
//! @param end      Points to nullptr on first call, will (point) out position
//! to read next from for subsequent reads of integers within one string (stream).
unsigned long read_ulong(char const* source, char** end = nullptr);

template <typename V> struct Subgraph
{
    V source_begin = 0;
    V source_end = std::numeric_limits<V>::max(); 
    V target_begin = 0;
    V target_end = std::numeric_limits<V>::max();
};

//! TODO: Currently we will remove one edge from the graph file due to a possible header line.
//!       This must either be (somehow) automatically determined or flagged by the user to fix
//!       the removal of one edge.
template <typename Vertex, typename EmplaceF, bool IsDirected, bool IsWeighted, bool IsDynamic = false, typename Timestamp = uint64_t, bool ExtractSubgraph = false>
Vertex read_graph_generic(std::istream& input, EmplaceF&& emplace, uint64_t const edge_count_max = std::numeric_limits<uint64_t>::max(), Subgraph<Vertex>&& subgraph = Subgraph<Vertex>{})
{
    std::string line;
    bool seen_header = false;
    unsigned long u = 0;
    unsigned long v = 0;
    float w = 1.;
    unsigned long t = 0;
    unsigned long n = 0;

    char const* string_source = nullptr;
    char* string_position = nullptr;

    for (uint64_t edge_counter = 0; std::getline(input, line) && edge_counter < edge_count_max;)
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        string_source = line.c_str();
        string_position = nullptr;

        u = read_ulong(string_source, &string_position);
        string_source = string_position;
        v = read_ulong(string_source, &string_position);

        if (u > n)
        {
            n = u;
        }
        else if (v > n)
        {
            n = v;
        }

        if constexpr (ExtractSubgraph)
        {
            if (u < subgraph.source_begin || u >= subgraph.source_end || v < subgraph.target_begin || v >= subgraph.target_end)
            {
                continue;
            }
        }

        if constexpr (IsWeighted)
        {
            string_source = string_position;
            w = read_ulong(string_source, &string_position);
        }

        if constexpr (IsDynamic)
        {
            string_source = string_position;
            t = read_ulong(string_source, &string_position);
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        if constexpr (IsDirected)
        {
            if constexpr (IsDynamic)
            {
                emplace(static_cast<Vertex>(u), static_cast<Vertex>(v), w, static_cast<Timestamp>(t));
            }
            else 
            {
                emplace(static_cast<Vertex>(u), static_cast<Vertex>(v), w);
            }

            ++edge_counter;
        }
        else 
        {
            if constexpr (IsDynamic)
            {
                emplace(static_cast<Vertex>(u), static_cast<Vertex>(v), w, static_cast<Timestamp>(t));
                emplace(static_cast<Vertex>(v), static_cast<Vertex>(u), w, static_cast<Timestamp>(t));
            }
            else 
            {
                emplace(static_cast<Vertex>(u), static_cast<Vertex>(v), w);
                emplace(static_cast<Vertex>(v), static_cast<Vertex>(u), w);
            }
            
            edge_counter  += 2;
        }
    }

    return ++n;
}

template <typename Vertex, typename EmplaceF, bool Directed, bool Weighted, bool Dynamic = false, typename Timestamp = uint64_t>
Vertex read_graph_generic(std::string const& path, EmplaceF&& emplace, uint64_t const edge_count_max = std::numeric_limits<uint64_t>::max())
{
    namespace fs = std::experimental::filesystem;

    fs::path graph_path(path);

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);
    return read_graph_generic<Vertex, EmplaceF, Directed, Weighted, Dynamic, Timestamp>(graph_input, std::move(emplace), edge_count_max);
}

template <typename Vertex, typename Label, typename F> void read_labels(std::istream& ins, F&& emplace)
{
    std::string line;
    int ret = 0;

    unsigned long u = 0;
    unsigned long l = 0;

    char const* string_source = nullptr;
    char* string_position = nullptr;

    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        string_source = line.c_str();
        string_position = nullptr;

        u = read_ulong(string_source, &string_position);
        string_source = string_position;
        l = read_ulong(string_source, &string_position);

        emplace(static_cast<Vertex>(u), static_cast<Label>(l));
    }
}

template <typename Label> std::vector<Label> read_graph_labels(std::istream& ins)
{
    std::string line;
    int ret = 0;
    int const retrieved_elements = 1;
    // construct with label 0 for idx 0 which will not be used
    std::vector<Label> graph_labels{ 0 };
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long label = 0;
        ret = sscanf(line.data(), "%lu", &label);

        if (!ret)
        {
            continue;
        }

        if (ret != retrieved_elements)
        {
            throw std::runtime_error("Parse error while reading label!");
        }

        graph_labels.push_back(label);
    }
    return graph_labels;
}

template <typename Vertex, typename F> void read_graph_idx(std::istream& ins, F&& emplace)
{
    std::string line;
    int ret = 0;
    int const retrieved_elements = 1;
    Vertex i = 0;
    // construct with label 0 for idx 0 which will not be used
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long idx = 0;
        ret = sscanf(line.data(), "%lu", &idx);

        if (!ret)
        {
            continue;
        }

        if (ret != retrieved_elements)
        {
            throw std::runtime_error("Parse error while reading labeled node!");
        }

        emplace(i, idx);

        ++i;
    }
}

} // namespace gdsb
