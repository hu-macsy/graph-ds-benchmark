#include <gdsb/graph.h>

#include <limits>

namespace gdsb
{

Weight invalid_weight() { return std::numeric_limits<Weight>::infinity(); }

// TODO: Write tests for gilbert_edges() and gilbert_edges_64()
Edges64 gilbert_edges_64(float const p, Vertex64 const n, std::mt19937& engine)
{
    Edges64 random_edges;
    std::uniform_real_distribution<float> distrib{ 0.0f, 1.0f };

    for (Vertex64 i = 0; i < n; ++i)
    {
        for (Vertex64 j = 0; j < n; ++j)
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
