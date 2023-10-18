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

template <bool value> class GraphParameter
{
public:
    static constexpr bool is() { return value; }
};

class Directed : private GraphParameter<true>
{
public:
    static constexpr bool is_directed() { return GraphParameter::is(); }
};

class Undirected : private GraphParameter<false>
{
public:
    static constexpr bool is_directed() { return GraphParameter::is(); }
};

class Weighted : private GraphParameter<true>
{
public:
    static constexpr bool is_weighted() { return GraphParameter::is(); }
};

class Unweighted : private GraphParameter<false>
{
public:
    static constexpr bool is_weighted() { return GraphParameter::is(); }
};

class Dynamic : private GraphParameter<true>
{
public:
    static constexpr bool is_dynamic() { return GraphParameter::is(); }
};

class Static : private GraphParameter<false>
{
public:
    static constexpr bool is_dynamic() { return GraphParameter::is(); }
};

enum class FileType
{
    edge_list,
    matrix_market
};

//! @param  file_type       Choose the FileType to read in.
template <FileType file_type, typename Directed = Undirected, typename Weighted = Unweighted, typename Dynamic = Static>
class GraphParameters
{
public:
    static constexpr bool is_directed() { return Directed::is_directed(); }
    static constexpr bool is_weighted() { return Weighted::is_weighted(); }
    static constexpr bool is_dynamic() { return Dynamic::is_dynamic(); }
    static constexpr FileType filetype() { return file_type; }
};

//! Some useful using directives for edge list input file types
using EdgeListDirectedWeightedStatic = GraphParameters<FileType::edge_list, Directed, Weighted, Static>;
using EdgeListDirectedWeightedDynamic = GraphParameters<FileType::edge_list, Directed, Weighted, Dynamic>;
using EdgeListDirectedUnweightedStatic = GraphParameters<FileType::edge_list, Directed, Unweighted, Static>;
using EdgeListDirectedUnweightedDynamic = GraphParameters<FileType::edge_list, Directed, Unweighted, Dynamic>;
using EdgeListUndirectedWeightedStatic = GraphParameters<FileType::edge_list, Undirected, Weighted, Static>;
using EdgeListUndirectedWeightedDynamic = GraphParameters<FileType::edge_list, Undirected, Weighted, Dynamic>;
using EdgeListUndirectedUnweightedStatic = GraphParameters<FileType::edge_list, Undirected, Unweighted, Static>;
using EdgeListUndirectedUnweightedDynamic = GraphParameters<FileType::edge_list, Undirected, Unweighted, Dynamic>;

//! Some useful using directives for matrix market input file types
using MatrixMarketDirectedWeightedStatic = GraphParameters<FileType::matrix_market, Directed, Weighted, Static>;
using MatrixMarketDirectedWeightedDynamic = GraphParameters<FileType::matrix_market, Directed, Weighted, Dynamic>;
using MatrixMarketDirectedUnweightedStatic = GraphParameters<FileType::matrix_market, Directed, Unweighted, Static>;
using MatrixMarketDirectedUnweightedDynamic = GraphParameters<FileType::matrix_market, Directed, Unweighted, Dynamic>;
using MatrixMarketUndirectedWeightedStatic = GraphParameters<FileType::matrix_market, Undirected, Weighted, Static>;
using MatrixMarketUndirectedWeightedDynamic = GraphParameters<FileType::matrix_market, Undirected, Weighted, Dynamic>;
using MatrixMarketUndirectedUnweightedStatic = GraphParameters<FileType::matrix_market, Undirected, Unweighted, Static>;
using MatrixMarketUndirectedUnweightedDynamic = GraphParameters<FileType::matrix_market, Undirected, Unweighted, Dynamic>;

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
    namespace fs = std::experimental::filesystem;

    fs::path graph_path(path);

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);
    return read_graph<Vertex, EmplaceF, GraphParameters, Timestamp>(graph_input, std::move(emplace), edge_count_max);
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
