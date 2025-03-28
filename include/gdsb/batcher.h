#pragma once

#include <gdsb/graph.h>

#include <algorithm>
#include <iterator>
#include <tuple>

namespace gdsb
{

template <class EdgeContainer> struct Batch
{
    typename EdgeContainer::const_iterator begin;
    typename EdgeContainer::const_iterator end;
};

// Use this class to create batches of edges.
template <class EdgeContainer> class Batcher
{
public:
    using EIt = typename EdgeContainer::iterator;
    using EItConst = typename EdgeContainer::const_iterator;
    using Batch_t = Batch<EdgeContainer>;

    Batcher(EIt begin, EIt end, size_t const batch_size)
    : m_batch_begin(begin)
    , m_end(end)
    , m_batch_size(batch_size)
    {
        if (batch_size < std::distance(begin, end))
        {
            m_batch_end = std::next(m_batch_begin, m_batch_size);
        }
        else
        {
            m_batch_end = m_end;
        }
    }

    Batch_t next(unsigned int const thread_count)
    {
        Batch_t batch{ m_batch_begin, m_batch_end };

        m_batch_begin = std::next(m_batch_begin, m_batch_size);

        if (m_batch_begin > m_end)
        {
            m_batch_begin = m_end;
            m_batch_end = m_end;
        }
        else
        {
            m_batch_end = std::next(m_batch_end, m_batch_size);
            if (m_batch_end > m_end)
            {
                m_batch_end = m_end;
            }
        }

        return batch;
    }

    template <typename Cmp> Batch_t next_sorted(Cmp&& cmp, unsigned int const thread_count)
    {
        // Intentionally not using tlx::stable_parallel_mergesort() since we do not need to
        // preserver the order of edges with equal source ID as in the original edges vector
        // tlx::parallel_mergesort(m_batch_begin, m_batch_end, cmp, thread_count);
        // TODO: We would like to sort with multiple threads, but TLX' parallel_mergesort
        //       seems to be broken.
        std::sort(m_batch_begin, m_batch_end, cmp);

        return next(thread_count);
    }

    EItConst const end() { return m_end; }
    size_t size() const { return m_batch_size; }

private:
    size_t m_batch_size{ 1 };
    EIt m_batch_begin;
    EIt m_batch_end;
    EIt m_end;
};

constexpr uint32_t count_of_batches(uint64_t const edge_count, uint64_t expected_batch_size)
{
    return edge_count / expected_batch_size;
}

constexpr size_t fair_batch_size(uint64_t const edge_count, uint64_t max_batch_size)
{
    if (edge_count <= max_batch_size)
    {
        return edge_count;
    }

    uint64_t const batch_count = edge_count / max_batch_size;
    uint64_t const remaining = edge_count % max_batch_size;
    uint64_t fair_size = max_batch_size + (remaining / batch_count);
    return fair_size;
}

constexpr std::pair<uint64_t, uint64_t>
fair_batch_offset(uint64_t const fair_size, uint64_t current_batch_num, uint64_t batches_count, uint64_t edge_count)
{
    uint64_t begin = current_batch_num * fair_size;

    bool const is_last_batch = (current_batch_num + 1) == batches_count;
    if (is_last_batch)
    {
        uint64_t const count = edge_count - begin;
        return std::make_pair(begin, count);
    }

    uint64_t const count = fair_size;
    return std::make_pair(begin, count);
}

constexpr uint64_t partition_batch_count(uint64_t const batch_count, uint32_t const partition_id, uint32_t const partition_size)
{
    uint64_t partition_batch_count = batch_count / partition_size;
    if (partition_id == partition_size - 1)
    {
        partition_batch_count += batch_count % partition_size;
    }

    return partition_batch_count;
}

inline uint64_t batch_offset(uint64_t const batch_count, uint32_t const partition_id, uint32_t const partition_size)
{
    return batch_count / partition_size * partition_id;
}

template <typename EdgeIt, typename EdgeContainer, typename V>
Batch<EdgeContainer>
thread_batch(EdgeIt batch_begin, EdgeIt batch_end, unsigned int thread_count, unsigned int thread_id, V invalid_vertex)
{
    size_t const batch_size = std::distance(batch_begin, batch_end);
    size_t const elements = batch_size / thread_count;
    if (elements == 0 || elements == batch_size)
    {
        bool const first_thread = thread_id == 0;
        if (first_thread)
        {
            return Batch<EdgeContainer>{ batch_begin, batch_end };
        }
        else
        {
            return Batch<EdgeContainer>{ batch_end, batch_end };
        }
    }

    size_t const position = thread_id * elements;

    EdgeIt start = std::min(batch_begin + position, batch_end);

    bool const last_thread = thread_id == (thread_count - 1);
    EdgeIt end = last_thread ? batch_end : std::min(start + elements, batch_end);

    if (start != end)
    {
        V const predecessor = (start == batch_begin) ? invalid_vertex : std::prev(start, 1)->source;

        while (start != end && predecessor == start->source)
        {
            std::advance(start, 1);
        }

        if (start != end)
        {
            for (V successor = (end == batch_end) ? invalid_vertex : end->source;
                 end != batch_end && successor == (end - 1)->source && end->source != predecessor; successor = end->source)
            {
                std::advance(end, 1);
            }
        }
    }

    return Batch<EdgeContainer>{ start, end };
}

} // namespace gdsb
