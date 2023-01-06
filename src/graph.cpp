#include <gdsb/graph.h>

#include <limits>

namespace gdsb
{

Weight invalid_weight() { return std::numeric_limits<Weight>::infinity(); }

Edge invalid_edge()
{
    using Vertex_type = decltype(Edge::source);
    return { invalid_vertex<Vertex_type>(), { invalid_vertex<Vertex_type>(), invalid_weight() } };
}

Target invalid_target()
{
    using Vertex_type = decltype(Target::vertex);
    return { invalid_vertex<Vertex_type>(), invalid_weight() };
}

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
