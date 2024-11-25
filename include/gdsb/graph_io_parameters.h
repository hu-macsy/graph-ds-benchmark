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

class Loop : private GraphParameter<true>
{
public:
    static constexpr bool loop() { return GraphParameter::is(); }
};

class NoLoop : private GraphParameter<false>
{
public:
    static constexpr bool loop() { return GraphParameter::is(); }
};

enum class FileType
{
    edge_list,
    matrix_market,
    binary
};

//! @param  file_type       Choose the FileType.
template <FileType file_type, typename DirectedT = Undirected, typename WeightedT = Unweighted, typename LoopT = Loop, typename DynamicT = Static>
class GraphParameters
{
public:
    static constexpr bool is_directed() { return DirectedT::is_directed(); }
    static constexpr bool is_weighted() { return WeightedT::is_weighted(); }
    static constexpr bool is_dynamic() { return DynamicT::is_dynamic(); }
    static constexpr bool loop() { return LoopT::loop(); }
    static constexpr FileType filetype() { return file_type; }
};

//! Some useful using directives for edge list input file types
using EdgeListDirectedWeightedLoopStatic = GraphParameters<FileType::edge_list, Directed, Weighted, Loop, Static>;
using EdgeListDirectedWeightedLoopDynamic = GraphParameters<FileType::edge_list, Directed, Weighted, Loop, Dynamic>;
using EdgeListDirectedWeightedNoLoopStatic = GraphParameters<FileType::edge_list, Directed, Weighted, NoLoop, Static>;
using EdgeListDirectedWeightedNoLoopDynamic = GraphParameters<FileType::edge_list, Directed, Weighted, NoLoop, Dynamic>;
using EdgeListDirectedUnweightedLoopStatic = GraphParameters<FileType::edge_list, Directed, Unweighted, Loop, Static>;
using EdgeListDirectedUnweightedLoopDynamic = GraphParameters<FileType::edge_list, Directed, Unweighted, Loop, Dynamic>;
using EdgeListDirectedUnweightedNoLoopStatic = GraphParameters<FileType::edge_list, Directed, Unweighted, NoLoop, Static>;
using EdgeListDirectedUnweightedNoLoopDynamic = GraphParameters<FileType::edge_list, Directed, Unweighted, NoLoop, Dynamic>;
using EdgeListUndirectedWeightedLoopStatic = GraphParameters<FileType::edge_list, Undirected, Weighted, Loop, Static>;
using EdgeListUndirectedWeightedLoopDynamic = GraphParameters<FileType::edge_list, Undirected, Weighted, Loop, Dynamic>;
using EdgeListUndirectedWeightedNoLoopStatic = GraphParameters<FileType::edge_list, Undirected, Weighted, NoLoop, Static>;
using EdgeListUndirectedWeightedNoLoopDynamic = GraphParameters<FileType::edge_list, Undirected, Weighted, NoLoop, Dynamic>;
using EdgeListUndirectedUnweightedLoopStatic = GraphParameters<FileType::edge_list, Undirected, Unweighted, Loop, Static>;
using EdgeListUndirectedUnweightedLoopDynamic = GraphParameters<FileType::edge_list, Undirected, Unweighted, Loop, Dynamic>;
using EdgeListUndirectedUnweightedNoLoopStatic = GraphParameters<FileType::edge_list, Undirected, Unweighted, NoLoop, Static>;
using EdgeListUndirectedUnweightedNoLoopDynamic = GraphParameters<FileType::edge_list, Undirected, Unweighted, NoLoop, Dynamic>;

//! Some useful using directives for matrix market input file types
using MatrixMarketDirectedWeightedLoopStatic = GraphParameters<FileType::matrix_market, Directed, Weighted, Loop, Static>;
using MatrixMarketDirectedWeightedLoopDynamic = GraphParameters<FileType::matrix_market, Directed, Weighted, Loop, Dynamic>;
using MatrixMarketDirectedWeightedNoLoopStatic = GraphParameters<FileType::matrix_market, Directed, Weighted, NoLoop, Static>;
using MatrixMarketDirectedWeightedNoLoopDynamic = GraphParameters<FileType::matrix_market, Directed, Weighted, NoLoop, Dynamic>;
using MatrixMarketDirectedUnweightedLoopStatic = GraphParameters<FileType::matrix_market, Directed, Unweighted, Loop, Static>;
using MatrixMarketDirectedUnweightedLoopDynamic = GraphParameters<FileType::matrix_market, Directed, Unweighted, Loop, Dynamic>;
using MatrixMarketDirectedUnweightedNoLoopStatic = GraphParameters<FileType::matrix_market, Directed, Unweighted, NoLoop, Static>;
using MatrixMarketDirectedUnweightedNoLoopDynamic = GraphParameters<FileType::matrix_market, Directed, Unweighted, NoLoop, Dynamic>;
using MatrixMarketUndirectedWeightedLoopStatic = GraphParameters<FileType::matrix_market, Undirected, Weighted, Loop, Static>;
using MatrixMarketUndirectedWeightedLoopDynamic = GraphParameters<FileType::matrix_market, Undirected, Weighted, Loop, Dynamic>;
using MatrixMarketUndirectedWeightedNoLoopStatic = GraphParameters<FileType::matrix_market, Undirected, Weighted, NoLoop, Static>;
using MatrixMarketUndirectedWeightedNoLoopDynamic = GraphParameters<FileType::matrix_market, Undirected, Weighted, NoLoop, Dynamic>;
using MatrixMarketUndirectedUnweightedLoopStatic = GraphParameters<FileType::matrix_market, Undirected, Unweighted, Loop, Static>;
using MatrixMarketUndirectedUnweightedLoopDynamic = GraphParameters<FileType::matrix_market, Undirected, Unweighted, Loop, Dynamic>;
using MatrixMarketUndirectedUnweightedNoLoopStatic =
    GraphParameters<FileType::matrix_market, Undirected, Unweighted, NoLoop, Static>;
using MatrixMarketUndirectedUnweightedNoLoopDynamic =
    GraphParameters<FileType::matrix_market, Undirected, Unweighted, NoLoop, Dynamic>;


//! Some useful using directives for binary format
using BinaryDirectedWeightedStatic = GraphParameters<FileType::binary, Directed, Weighted, NoLoop, Static>;
using BinaryDirectedWeightedDynamic = GraphParameters<FileType::binary, Directed, Weighted, NoLoop, Dynamic>;
using BinaryDirectedUnweightedStatic = GraphParameters<FileType::binary, Directed, Unweighted, NoLoop, Static>;
using BinaryDirectedUnweightedDynamic = GraphParameters<FileType::binary, Directed, Unweighted, NoLoop, Dynamic>;
using BinaryUndirectedWeightedStatic = GraphParameters<FileType::binary, Undirected, Weighted, NoLoop, Static>;
using BinaryUndirectedWeightedDynamic = GraphParameters<FileType::binary, Undirected, Weighted, NoLoop, Dynamic>;
using BinaryUndirectedUnweightedStatic = GraphParameters<FileType::binary, Undirected, Unweighted, NoLoop, Static>;
using BinaryUndirectedUnweightedDynamic = GraphParameters<FileType::binary, Undirected, Unweighted, NoLoop, Dynamic>;

struct alignas(8) BinaryGraphHeaderIdentifier
{
    char identifier[4] = { 'G', 'D', 'S', 'B' };
    uint8_t version = 3;
};

struct alignas(8) BinaryGraphHeaderMetaDataV3
{
    uint64_t vertex_count = 2u;
    uint64_t edge_count = 1u;
    uint8_t vertex_id_byte_size = 4u;
    uint8_t weight_byte_size = 4u;
    uint8_t timestamp_byte_size = 4u;
    bool directed = false;
    bool weighted = false;
    bool dynamic = false;
};

} // namespace gdsb
