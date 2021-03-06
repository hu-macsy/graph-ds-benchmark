#pragma once

#include <algorithm>
#include <gdsb/sort_permutation.h>
#include <random>
#include <vector>

namespace gdsb
{

using Weight = float;
using Vertex = unsigned int;
using Degree = unsigned int;

struct Target
{
    Vertex vertex;
    Weight weight;
};

struct Edge
{
    Vertex source;
    Target target;
};

Weight invalid_weight();
Vertex invalid_vertex();
Edge invalid_edge();
Target invalid_target();

using Edges = std::vector<Edge>;
using Targets = std::vector<Target>;
using Timestamp = unsigned int;
using Timestamps = std::vector<Timestamp>;

gdsb::Vertex max_nnz(Edges const& edges);
unsigned int vertex_count(Edges const& edges);

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
