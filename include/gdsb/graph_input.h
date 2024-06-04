#pragma once

#include <gdsb/graph.h>
#include <gdsb/graph_io_parameters.h>

#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
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

//! Reads in the input expecting a graph file to be streamed which can contain
//! comments using characters % or #.
//!
//! Supported graph file formats:
//! - edge list aka ".edges" using FileType::edge_list
//! - market matrix aka ".mtx" using Filetype::matrix_market
//!
//! It is expected that the first valid edge line, and all following contain v
//! (0) and u (1) separated by space(s). All follow up data points such as
//! weight (2), or timestamp (4) will also be expected to be separated by spaces
//! but only if the template parameters are set accordingly.
//!
//! Use the GraphParameters template parameter for specification of the input
//! graphs parameter such as what file format to read, and if it is directed,
//! weighted, dynamic.
//!
//! @param  input           The graph file input stream.
//! @param  emplace         An emplace function that will be called passing u,
//!                         v, (w, t) to emplace the read data points e.g. in a
//!                         data structure. But emplace() can also be used to
//!                         process the passed data points in any other way. If
//!                         reading a directed graph, emplace() will be called
//!                         once with emplace(u, v, ...), if undirected
//!                         emplace() will be called with 1st emplace(u, v,
//!                         ...), 2nd with emplace(v, u, ...).
//! @param  edge_count_max  The maximum count of edges to read from. Set to max
//!                         if not specified.
//! @param  subgraph        A subgraph to extract from the file. Use default or
//!                         any other Subgraph object if not specified by
//!                         ExtractSubgraph.
template <typename Vertex, typename EmplaceF, typename GraphParameters = GraphParameters<FileType::edge_list>, typename Timestamp = uint64_t, bool ExtractSubgraph = false>
std::tuple<Vertex, uint64_t> read_graph(std::istream& input,
                                        EmplaceF&& emplace,
                                        uint64_t const edge_count_max = std::numeric_limits<uint64_t>::max(),
                                        Subgraph<Vertex>&& subgraph = Subgraph<Vertex>{})
{
    std::string line;

    if constexpr (GraphParameters::filetype() == FileType::edge_list)
    {
        bool continue_reading = true;
        while (continue_reading && std::getline(input, line))
        {
            continue_reading = (line.front() == '%' || line.front() == '#');
        }
    }
    else if constexpr (GraphParameters::filetype() == FileType::matrix_market)
    {
        bool continue_reading = true;
        while (continue_reading && std::getline(input, line))
        {
            continue_reading = (line.front() == '%' || line.front() == '#');
        }

        // This will discard the first line including parameter N and M
        // TODO: perhaps we want to make sure that these are actually 3 integers?
        std::getline(input, line);
    }

    unsigned long u = 0;
    unsigned long v = 0;
    float w = 1.;
    unsigned long t = 0;
    unsigned long n = 0;

    char const* string_source = nullptr;
    char* string_position = nullptr;

    uint64_t edge_counter = 0;
    do
    {
        if (line.empty()) continue;

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

        if constexpr (GraphParameters::is_weighted())
        {
            string_source = string_position;
            w = read_ulong(string_source, &string_position);
        }

        if constexpr (GraphParameters::is_dynamic())
        {
            string_source = string_position;
            t = read_ulong(string_source, &string_position);
        }

        if constexpr (GraphParameters::is_directed())
        {
            if constexpr (GraphParameters::is_dynamic())
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
            if constexpr (GraphParameters::is_dynamic())
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
    } while (std::getline(input, line) && edge_counter < edge_count_max);

    return { ++n, edge_counter };
}

template <typename Vertex, typename EmplaceF, typename GraphParameters = GraphParameters<FileType::edge_list>, typename Timestamp = uint64_t>
std::tuple<Vertex, uint64_t>
read_graph(std::string const& path, EmplaceF&& emplace, uint64_t const edge_count_max = std::numeric_limits<uint64_t>::max())
{
    namespace fs = std::filesystem;

    fs::path graph_path(path);

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);
    return read_graph<Vertex, EmplaceF, GraphParameters, Timestamp>(graph_input, std::move(emplace), edge_count_max);
}

inline BinaryGraphHeaderMetaDataV1 read_binary_graph_header(std::ifstream& input)
{
    BinaryGraphHeaderIdentifier id;
    input.read(reinterpret_cast<char*>(&id), sizeof(BinaryGraphHeaderIdentifier));

    GraphParameters<FileType::binary> graph_parameters;

    switch (id.version)
    {
    case 1:
    {
        if (!std::strcmp(id.identifier, "GDSB"))
        {
            throw std::logic_error(std::string("Binary graph file has wrong identifier: ") + std::string(id.identifier));
        }

        BinaryGraphHeaderMetaDataV1 meta_data;
        input.read(reinterpret_cast<char*>(&meta_data), sizeof(BinaryGraphHeaderMetaDataV1));

        return meta_data;
    }
    default:
        throw std::logic_error("Binary graph version not supported: " + std::to_string(id.version));
    }
}

template <typename Header, typename ReadF>
std::tuple<Vertex64, uint64_t> read_binary_graph(std::ifstream& input, Header const& header, ReadF&& read)
{
    bool continue_reading = true;
    uint64_t edge_count = header.edge_count;

    for (uint64_t e = 0; e < edge_count && input.is_open() && continue_reading; ++e)
    {
        continue_reading = read(input);
    }

    return std::make_tuple(header.vertex_count, header.edge_count);
}

uint64_t partition_edge_count(uint64_t total_edge_count, uint32_t partition_id, uint32_t partition_size);

template <typename ReadF>
std::tuple<Vertex64, uint64_t> read_binary_graph_partition(std::ifstream& input,
                                                           BinaryGraphHeaderMetaDataV1 const& data,
                                                           ReadF&& read,
                                                           size_t edge_size_in_bytes,
                                                           uint32_t const partition_id,
                                                           uint32_t const partition_size)
{
    assert(partition_size > 0);

    uint64_t const edge_count = partition_edge_count(data.edge_count, partition_id, partition_size);

    size_t const offset = [&]()
    {
        if (partition_id != uint32_t(partition_size - 1))
        {
            return edge_count * partition_id;
        }
        else
        {
            uint64_t partition_edge_count = data.edge_count / partition_size;
            return partition_edge_count * partition_id;
        }
    }();
    input.seekg(offset * edge_size_in_bytes, std::ios_base::cur);
    bool continue_reading = true;
    for (uint64_t e = 0; e < edge_count && input.is_open() && continue_reading; ++e)
    {
        continue_reading = read(input);
    }

    return std::make_tuple(data.vertex_count, edge_count);
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
