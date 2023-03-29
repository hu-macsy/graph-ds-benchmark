#pragma once

#include <gdsb/graph.h>

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>

namespace gdsb
{

template <typename Vertex, typename F> void read_graph_unweighted(std::istream& ins, F&& emplace)
{
    std::string line;
    bool seen_header = false;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long v = 0;
        auto ret = sscanf(line.data(), "%lu %lu", &u, &v);
        if (!ret)
        {
            continue;
        }

        if (ret != 2)
        {
            throw std::runtime_error("Parse error while reading input graph!");
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        emplace(static_cast<Vertex>(u), static_cast<Vertex>(v));
    }
}

template <typename V, typename F, typename Weight_f>
void read_directed_graph(std::istream& ins, bool const weighted, F&& emplace, Weight_f&& weight_f)
{
    std::string line;
    bool seen_header = false;
    int ret = 0;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long v = 0;
        float w = 0.;

        if (weighted)
        {
            ret = sscanf(line.data(), "%lu %lu %f", &u, &v, &w);
            if (ret != 3)
            {
                throw std::runtime_error("Parse error while reading input graph!");
            }
        }
        else
        {
            ret = sscanf(line.data(), "%lu %lu", &u, &v);
            if (ret != 2)
            {
                throw std::runtime_error("Parse error while reading input graph!");
            }
        }

        if (!ret)
        {
            continue;
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        using Weight_type = std::decay_t<decltype(weight_f())>;
        Weight_type const weight_f_result = [&]() { return weighted ? w : weight_f(); }();
        emplace(static_cast<V>(u), static_cast<V>(v), weight_f_result);
    }
}

// TODO: Further optimisation of reading in graphs by manually parsing integers instead of
//       using sscanf().
//
// TODO: Currently we will remove one edge from the graph file due to a possible header line.
//       This must either be (somehow) automatically determined or flagged by the user to fix
//       the removal of one edge.
template <typename Vertex, typename F> void read_graph_unweighted(std::string path, F&& emplace)
{
    namespace fs = std::experimental::filesystem;

    fs::path graph_path(std::move(path));

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);
    read_graph_unweighted<Vertex, F>(graph_input, std::move(emplace));
}

template <typename Vertex, typename T, typename F>
void read_temporal_graph(std::istream& ins, bool const weighted, F&& emplace)
{
    std::string line;
    bool seen_header = false;
    int ret = 0;
    int const retrieved_elements = weighted ? 4 : 3;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long v = 0;
        unsigned long t = 0;
        float w = 0.;

        if (weighted)
        {
            ret = sscanf(line.data(), "%lu %lu %f %lu", &u, &v, &w, &t);
        }
        else
        {
            ret = sscanf(line.data(), "%lu %lu %lu", &u, &v, &t);
        }

        if (!ret)
        {
            continue;
        }

        if (ret != retrieved_elements)
        {
            throw std::runtime_error("Parse error while reading input graph!");
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        emplace(static_cast<Vertex>(u), static_cast<Vertex>(v), static_cast<T>(t));
    }
}

template <typename Vertex, typename EdgeVector, typename TStamps, typename T, typename F>
TimestampedEdges<EdgeVector, TStamps> read_temporal_graph(std::string path, bool const weighted, F&& emplace)
{
    namespace fs = std::experimental::filesystem;

    fs::path graph_path(std::move(path));

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);

    TimestampedEdges<EdgeVector, TStamps> timestamped_edges;
    read_temporal_graph<Vertex, T>(graph_input, weighted,
                                   [&](unsigned int u, unsigned int v, unsigned int t)
                                   {
                                       emplace(timestamped_edges.edges, u, v);
                                       timestamped_edges.timestamps.push_back(t);
                                   });

    return timestamped_edges;
}

template <typename Vertex, typename F, typename Weight_f>
void read_undirected_graph_unweighted(std::istream& ins, F&& emplace, Weight_f&& weight_f)
{
    std::string line;
    bool seen_header = false;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long v = 0;
        auto ret = sscanf(line.data(), "%lu %lu", &u, &v);
        if (!ret)
        {
            continue;
        }

        if (ret != 2)
        {
            throw std::runtime_error("Parse error while reading input graph!");
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        using Weight_type = std::decay_t<decltype(weight_f())>;
        Weight_type const w = weight_f();
        emplace(static_cast<Vertex>(u), static_cast<Vertex>(v), w);
        emplace(static_cast<Vertex>(v), static_cast<Vertex>(u), w);
    }
}

template <typename V, typename F, typename Weight_f>
void read_undirected_graph(std::istream& ins, bool const weighted, F&& emplace, Weight_f&& weight_f)
{
    std::string line;
    bool seen_header = false;
    int ret = 0;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long v = 0;
        float w = 0.;

        if (weighted)
        {
            ret = sscanf(line.data(), "%lu %lu %f", &u, &v, &w);
            if (ret != 3)
            {
                throw std::runtime_error("Parse error while reading input graph!");
            }
        }
        else
        {
            ret = sscanf(line.data(), "%lu %lu", &u, &v);
            if (ret != 2)
            {
                throw std::runtime_error("Parse error while reading input graph!");
            }
        }

        if (!ret)
        {
            continue;
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        using Weight_type = std::decay_t<decltype(weight_f())>;
        Weight_type const weight_f_result = [&]() { return weighted ? w : weight_f(); }();
        emplace(static_cast<V>(u), static_cast<V>(v), weight_f_result);
        emplace(static_cast<V>(v), static_cast<V>(u), weight_f_result);
    }
}

template <typename Vertex, typename F, typename Weight_f>
void read_undirected_graph_unweighted(std::string path, F&& emplace, Weight_f&& weight_f)
{
    namespace fs = std::experimental::filesystem;

    fs::path graph_path(std::move(path));

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);
    read_undirected_graph_unweighted<Vertex, F, Weight_f>(graph_input, std::move(emplace), std::move(weight_f));
}

template <typename V, typename T, typename F, typename Weight_f>
void read_temporal_graph(std::istream& ins, bool const weighted, bool const directed, F&& emplace, Weight_f&& weight_f)
{
    std::string line;
    bool seen_header = false;
    int ret = 0;
    int const retrieved_elements = weighted ? 4 : 3;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long v = 0;
        unsigned long t = 0;
        float w = 0.;

        if (weighted)
        {
            ret = sscanf(line.data(), "%lu %lu %f %lu", &u, &v, &w, &t);
        }
        else
        {
            ret = sscanf(line.data(), "%lu %lu %lu", &u, &v, &t);
        }

        if (!ret)
        {
            continue;
        }

        if (ret != retrieved_elements)
        {
            throw std::runtime_error("Parse error while reading input graph!");
        }

        if (!seen_header)
        {
            seen_header = true;
            continue;
        }

        using Weight_type = std::decay_t<decltype(weight_f())>;
        Weight_type const weight_f_result = [&]() { return weighted ? w : weight_f(); }();

        emplace(V(u), V(v), weight_f_result, T(t));

        if (!directed)
        {
            emplace(V(v), V(u), weight_f_result, T(t));
        }
    }
}

template <typename Vertex, typename EdgeVector, typename Timestamps, typename F, typename Weight_f>
TimestampedEdges<EdgeVector, Timestamps>
read_temporal_undirected_graph(std::string path, bool const weighted, F&& emplace, Weight_f&& weight_f)
{
    namespace fs = std::experimental::filesystem;

    fs::path graph_path(std::move(path));

    if (!fs::exists(graph_path))
    {
        throw std::runtime_error("Path to graph does not exist!");
    }

    std::ifstream graph_input(graph_path);

    TimestampedEdges<EdgeVector, Timestamps> timestamped_edges;
    read_temporal_graph<Vertex, typename Timestamps::value_type>(
        graph_input, weighted, false,
        [&](uint64_t u, uint64_t v, float w, typename Timestamps::value_type t)
        {
            emplace(timestamped_edges.edges, u, v, w);
            timestamped_edges.timestamps.push_back(t);
        },
        std::move(weight_f));

    return timestamped_edges;
}

template <typename Vertex, typename Label, typename F> void read_labels(std::istream& ins, F&& emplace)
{
    std::string line;
    int ret = 0;
    int const retrieved_elements = 2;
    while (std::getline(ins, line))
    {
        if (line.empty()) continue;
        if (line.front() == '%') continue;
        if (line.front() == '#') continue;

        unsigned long u = 0;
        unsigned long l = 0;

        ret = sscanf(line.data(), "%lu %lu", &u, &l);


        if (!ret)
        {
            continue;
        }

        if (ret != retrieved_elements)
        {
            throw std::runtime_error("Parse error while reading labeled node!");
        }

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
