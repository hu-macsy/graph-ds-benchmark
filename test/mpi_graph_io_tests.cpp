#include <catch2/catch_test_macros.hpp>

#include "test_graph.h"

#include <gdsb/mpi_graph_io.h>

using namespace gdsb;

TEST_CASE("Open MPI File")
{
    constexpr int required_thread_support = MPI_THREAD_MULTIPLE;
    int provided_thread_support = required_thread_support;

    int dumy_argc = 0;
    char** dummy_argv;
    MPI_Init_thread(&dumy_argc, &dummy_argv, required_thread_support, &provided_thread_support);
    assert(required_thread_support == provided_thread_support);

    SECTION("Does not throw") { CHECK_NOTHROW(gdsb::mpi::open_file(graph_path + unweighted_directed_graph_enzymes)); }

    MPI_Finalize();
}
