#include <gdsb/graph.h>

#include <limits>

namespace gdsb
{

Weight invalid_weight() { return std::numeric_limits<Weight>::infinity(); }

Vertex invalid_vertex() { return std::numeric_limits<Vertex>::max(); }

Edge invalid_edge() { return { invalid_vertex(), { invalid_vertex(), invalid_weight() } }; }

Target invalid_target() { return { invalid_vertex(), invalid_weight() }; }

Vertex max_nnz(Edges const& edges)
{
    std::vector<Vertex> ids;

    for (Edge const& e : edges)
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

unsigned int vertex_count(Edges const& edges)
{
    // Determine n as the maximal node ID.
    unsigned int n = 0;
    for (Edge const& edge : edges)
    {
        unsigned int const vertex = std::max(edge.source, edge.target.vertex);
        n = std::max(n, vertex);
    }
    n++;

    return n;
}

Edges gilbert_edges(float const p, unsigned int const n, std::mt19937& engine)
{
    Edges random_edges;
    std::uniform_real_distribution<float> distrib{ 0.0f, 1.0f };

    for (unsigned int i = 0; i < n; ++i)
    {
        for (unsigned int j = 0; j < n; ++j)
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

} // namespace gdsb
