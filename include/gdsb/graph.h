#pragma once

#include <algorithm>
#include <gdsb/sort_permutation.h>
#include <random>
#include <vector>

namespace gdsb
{

using Weight = float;

using Vertex32 = unsigned int;
using Vertex64 = uint64_t;
using Degree32 = unsigned int;
using Degree64 = uint64_t;

template <typename VertexT, typename WeightT> struct Target
{
    VertexT vertex;
    WeightT weight;
};


template <typename VertexT, typename TargetT> struct Edge
{
    VertexT source;
    TargetT target;
};

Weight invalid_weight();

template <typename V> V invalid_vertex() { return std::numeric_limits<V>::max(); }

template <typename EdgeT> EdgeT invalid_edge()
{
    using Vertex_type = decltype(EdgeT::source);
    return { invalid_vertex<Vertex_type>(), { invalid_vertex<Vertex_type>(), invalid_weight() } };
}

template <typename TargetT> TargetT invalid_target()
{
    using Vertex_type = decltype(TargetT::vertex);
    return { invalid_vertex<Vertex_type>(), invalid_weight() };
}

using Target32 = Target<Vertex32, Weight>;
using Edge32 = Edge<Vertex32, Target32>;
using Edges32 = std::vector<Edge32>;

using Target64 = Target<Vertex64, Weight>;
using Edge64 = Edge<Vertex64, Target64>;
using Edges64 = std::vector<Edge64>;

using Targets32 = std::vector<Target32>;
using Targets64 = std::vector<Target64>;

using Timestamp32 = uint32_t;
using Timestamp64 = uint64_t;

using Timestamps32 = std::vector<Timestamp32>;
using Timestamps64 = std::vector<Timestamp64>;

template <typename EdgesT> auto max_nnz(EdgesT const& edges)
{
    std::vector<EdgesT> ids;

    for (auto const& e : edges)
    {
        if (e.source >= ids.size())
        {
            ids.resize(e.source + 1);
            ids[e.source] = 1;
        }
        else
        {
            ids[e.source]++;
        }
    }

    return *std::max_element(std::begin(ids), std::end(ids));
}

template <typename Edges_t> decltype(Edges_t::value_type::source) vertex_count(Edges_t const& edges)
{
    using Vertex_t = decltype(Edges_t::value_type::source);

    // Determine n as the maximal node ID.
    Vertex_t n = 0;
    using Edge_type = typename Edges_t::value_type;
    for (Edge_type const& edge : edges)
    {
        Vertex_t const vertex = std::max(edge.source, edge.target.vertex);
        n = std::max(n, vertex);
    }
    n++;

    return n;
}

template <typename Edges, typename TStamps> struct TimestampedEdges
{
    Edges edges;
    TStamps timestamps;
};

template <typename E, typename EdgeIt> void sort(EdgeIt begin, EdgeIt end)
{
    auto sort_cmp = [](E const& a, E const& b) { return a.source < b.source; };
    std::sort(begin, end, sort_cmp);
}

template <typename Edges, typename TStamps, typename T>
TimestampedEdges<Edges, TStamps>& sort(TimestampedEdges<Edges, TStamps>& t_edges)
{
    TStamps& timestamps = t_edges.timestamps;

    auto p = sort_permutation(timestamps, [](T const& a, T const& b) { return a < b; });
    apply_permutation_in_place(t_edges.edges, p);
    apply_permutation_in_place(timestamps, p);

    return t_edges;
}

template <typename VertexT, typename EdgesT> EdgesT gilbert_edges(float const p, VertexT const n, std::mt19937& engine)
{
    EdgesT random_edges;
    std::uniform_real_distribution<float> distrib{ 0.0f, 1.0f };

    for (VertexT i = 0; i < n; ++i)
    {
        for (VertexT j = 0; j < n; ++j)
        {
            // j != i -> no loops in simple directed graphs
            if (j != i && distrib(engine) <= p)
            {
                random_edges.push_back({ i, j, 1.f });
            }
        }
    }

    return random_edges;
}

Edges64 gilbert_edges_64(float const p, Vertex64 const n, std::mt19937& engine);

} // namespace gdsb
