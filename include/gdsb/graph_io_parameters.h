#pragma once

namespace gdsb
{
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
    matrix_market,
    binary
};

//! @param  file_type       Choose the FileType.
template <FileType file_type, typename DirectedT = Undirected, typename WeightedT = Unweighted, typename DynamicT = Static>
class GraphParameters
{
public:
    static constexpr bool is_directed() { return DirectedT::is_directed(); }
    static constexpr bool is_weighted() { return WeightedT::is_weighted(); }
    static constexpr bool is_dynamic() { return DynamicT::is_dynamic(); }
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

//! Some useful using directives for binary format
using BinaryDirectedWeightedStatic = GraphParameters<FileType::binary, Directed, Weighted, Static>;
using BinaryDirectedWeightedDynamic = GraphParameters<FileType::binary, Directed, Weighted, Dynamic>;
using BinaryDirectedUnweightedStatic = GraphParameters<FileType::binary, Directed, Unweighted, Static>;
using BinaryDirectedUnweightedDynamic = GraphParameters<FileType::binary, Directed, Unweighted, Dynamic>;
using BinaryUndirectedWeightedStatic = GraphParameters<FileType::binary, Undirected, Weighted, Static>;
using BinaryUndirectedWeightedDynamic = GraphParameters<FileType::binary, Undirected, Weighted, Dynamic>;
using BinaryUndirectedUnweightedStatic = GraphParameters<FileType::binary, Undirected, Unweighted, Static>;
using BinaryUndirectedUnweightedDynamic = GraphParameters<FileType::binary, Undirected, Unweighted, Dynamic>;

struct alignas(8) BinaryGraphHeaderIdentifier
{
    char identifier[4] = { 'G', 'D', 'S', 'B' };
    uint32_t version = 1;
};

struct alignas(8) BinaryGraphHeaderMetaDataV1
{
    uint64_t vertex_count = 2;
    uint64_t edge_count = 1;
    bool directed = false;
    bool weighted = false;
    bool dynamic = false;
};

} // namespace gdsb
