#pragma once

#include <algorithm>
#include <gdsb/sort_permutation.h>
#include <random>
#include <vector>

namespace gdsb
{

using Weight = float;

using Vertex = unsigned int;
using Vertex64 = uint64_t;
using Degree = unsigned int;
using Degree64 = uint64_t;

struct Target
{
    Vertex vertex;
    Weight weight;
};

template <typename V, typename W> struct Target_t
{
    V vertex;
    W weight;
};

using Target64 = gdsb::Target_t<gdsb::Vertex64, gdsb::Weight>;

struct Edge
{
    Vertex source;
    Target target;
};

template <typename V, typename T> struct Edge_t
{
    V source;
    T target;
};

using Edge64 = gdsb::Edge_t<gdsb::Vertex64, Target64>;

Weight invalid_weight();
Vertex invalid_vertex();
Edge invalid_edge();
Target invalid_target();

using Edges = std::vector<Edge>;
using Edges64 = std::vector<Edge64>;
using Targets = std::vector<Target>;
using Timestamp = unsigned int;
using Timestamps = std::vector<Timestamp>;

gdsb::Vertex max_nnz(Edges const& edges);

template <typename Vertex_t, typename Edges_t, typename Edge_t> Vertex_t vertex_count(Edges_t const& edges)
{
    // Determine n as the maximal node ID.
    unsigned int n = 0;
    for (Edge_t const& edge : edges)
    {
        unsigned int const vertex = std::max(edge.source, edge.target.vertex);
        n = std::max(n, vertex);
    }
    n++;

    return n;
}

template <typename Edges> struct TimestampedEdges
{
    Edges edges;
    Timestamps timestamps;
};

template <typename E, typename EdgeIt> void sort(EdgeIt begin, EdgeIt end)
{
    // sorting the edges for later purpose: inserting them into the batches
    auto sort_cmp = [](E const& a, E const& b) { return a.source < b.source; };
    std::sort(begin, end, sort_cmp);
}

template <typename Edges> TimestampedEdges<Edges>& sort(TimestampedEdges<Edges>& t_edges)
{
    Timestamps& timestamps = t_edges.timestamps;

    auto p = sort_permutation(timestamps, [](Timestamp const& a, Timestamp const& b) { return a < b; });
    apply_permutation_in_place(t_edges.edges, p);
    apply_permutation_in_place(timestamps, p);

    return t_edges;
}

Edges gilbert_edges(float const p, unsigned int const n, std::mt19937& engine);

} // namespace gdsb
