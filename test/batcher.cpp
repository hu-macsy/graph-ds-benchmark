#include <catch2/catch_test_macros.hpp>

#include <gdsb/batcher.h>
#include <gdsb/graph.h>

using namespace gdsb;

TEST_CASE("Batcher")
{
    Edges edges{ { 0, { 1, 1.f } }, { 0, { 2, 1.f } }, { 0, { 3, 1.f } }, { 1, { 2, 1.f } },
                 { 1, { 3, 1.f } }, { 2, { 5, 1.f } }, { 2, { 6, 1.f } } };

    CHECK(edges.size() == 7);

    SECTION("Unsorted batch")
    {
        Batcher<Edges> batcher(std::begin(edges), std::end(edges), 3);
        Batch<Edges> batch = batcher.next(1);

        CHECK(batch.begin->source == edges[0].source);
        CHECK(batch.begin->target.vertex == edges[0].target.vertex);
    }
}
