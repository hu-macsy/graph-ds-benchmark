#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/batcher.h>
#include <gdsb/graph.h>
#include <gdsb/graph_input.h>

using namespace gdsb;

TEST_CASE("Batcher")
{
    WeightedEdges32 edges{ { 0, { 1, 1.f } }, { 0, { 2, 1.f } }, { 0, { 3, 1.f } }, { 1, { 2, 1.f } },
                           { 1, { 3, 1.f } }, { 2, { 5, 1.f } }, { 2, { 6, 1.f } } };

    CHECK(edges.size() == 7);

    SECTION("Unsorted batch")
    {
        Batcher<WeightedEdges32> batcher(std::begin(edges), std::end(edges), 3);
        Batch<WeightedEdges32> batch = batcher.next(1);

        CHECK(batch.begin->source == edges[0].source);
        CHECK(batch.begin->target.vertex == edges[0].target.vertex);
    }
}

TEST_CASE("partition_batch_count, on enzymes graph")
{
    std::ifstream binary_graph(graph_path + directed_unweighted_graph_enzymes_bin);
    BinaryGraphHeader header = read_binary_graph_header(binary_graph);

    SECTION("partition size 2")
    {
        uint32_t partition_size = 2;

        uint32_t partition_id = 0;
        uint64_t edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 84);

        partition_id = 1;
        edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 84);
    }

    SECTION("partition size 5")
    {
        uint32_t partition_size = 5;

        uint32_t partition_id = 0;
        uint64_t edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 1;
        edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 2;
        edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 3;
        edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 33);

        partition_id = 4;
        edge_count = partition_batch_count(header.edge_count, partition_id, partition_size);
        CHECK(edge_count == 36);
    }
}

TEST_CASE("count_of_batches()")
{
    SECTION("simple")
    {
        uint64_t constexpr edge_count = 30;
        uint64_t constexpr max_batch_size = 10;
        uint32_t cob = count_of_batches(edge_count, max_batch_size);

        CHECK(cob == 3);
    }

    SECTION("simple with rest")
    {
        uint64_t constexpr edge_count = 33;
        uint64_t constexpr max_batch_size = 10;
        uint32_t cob = count_of_batches(edge_count, max_batch_size);

        CHECK(cob == 3);
    }

    SECTION("simple with max rest")
    {
        uint64_t constexpr edge_count = 39;
        uint64_t constexpr max_batch_size = 10;
        uint32_t cob = count_of_batches(edge_count, max_batch_size);

        CHECK(cob == 3);
    }
}

TEST_CASE("fair_batch_size()")
{
    SECTION("simple")
    {
        uint64_t constexpr edge_count = 30;
        uint64_t constexpr max_batch_size = 10;
        uint32_t fbs = fair_batch_size(edge_count, max_batch_size);

        CHECK(fbs == 10);
    }

    SECTION("simple with rest")
    {
        uint64_t constexpr edge_count = 33;
        uint64_t constexpr max_batch_size = 10;
        uint32_t fbs = fair_batch_size(edge_count, max_batch_size);

        CHECK(fbs == 11);
    }

    SECTION("simple with max rest")
    {
        uint64_t constexpr edge_count = 39;
        uint64_t constexpr max_batch_size = 10;
        uint32_t fbs = fair_batch_size(edge_count, max_batch_size);

        CHECK(fbs == 13);
    }

    SECTION("worst case, 2 remaining unincluded ")
    {
        uint64_t constexpr edge_count = 38;
        uint64_t constexpr max_batch_size = 10;
        uint32_t fbs = fair_batch_size(edge_count, max_batch_size);

        CHECK(fbs == 12);
    }
}
