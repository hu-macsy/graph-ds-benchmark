#include <catch2/catch_test_macros.hpp>

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "test_graph.h"

#include <gdsb/mpi_graph_io.h>

#include <filesystem>

class mpiInitListener : public Catch::EventListenerBase
{
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override
    {
        constexpr int required_thread_support = MPI_THREAD_MULTIPLE;
        int provided_thread_support = required_thread_support;

        int dumy_argc = 0;
        char** dummy_argv;
        MPI_Init_thread(&dumy_argc, &dummy_argv, required_thread_support, &provided_thread_support);
        assert(required_thread_support == provided_thread_support);
    }

    void testRunEnded(Catch::TestRunStats const& testRunStats) override { MPI_Finalize(); }
};

CATCH_REGISTER_LISTENER(mpiInitListener)

TEST_CASE("Open MPI File")
{
    SECTION("Does not throw using valid path opening regular file.")
    {
        std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes);
        CHECK_NOTHROW(gdsb::mpi::open_file(file_path));
    }

    SECTION("Does not throw using valid path opening binary file.")
    {
        std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes_bin);
        CHECK_NOTHROW(gdsb::mpi::open_file(file_path));
    }

    SECTION("Throws using invalid path.")
    {
        std::string invalid_path = "this/is/an/invalid/path.bin";
        std::filesystem::path file_path(invalid_path.c_str());
        CHECK_THROWS(gdsb::mpi::open_file(file_path));
    }
}

TEST_CASE("Read Small Weighted Temporal Binary File Header Information")
{
    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);

    MPI_File input;
    CHECK_NOTHROW(input = gdsb::mpi::open_file(file_path));

    gdsb::BinaryGraphHeaderMetaDataV1 data = gdsb::mpi::read_binary_graph_header(input);

    CHECK(data.directed == true);
    CHECK(data.weighted == true);
    CHECK(data.dynamic == true);
    CHECK(data.vertex_count == 7);
    CHECK(data.edge_count == 6);
}

TEST_CASE("Read Small Weighted Temporal Binary File")
{
    gdsb::TimestampedEdges32 timestamped_edges;
    auto read_f = [&](MPI_File input)
    {
        timestamped_edges.push_back(std::tuple<gdsb::Edge32, gdsb::Timestamp32>{});

        MPI_Status status;
        MPI_File_read(input, &std::get<0>(timestamped_edges.back()).source, 1, MPI_INT32_T, &status);
        MPI_File_read(input, &std::get<0>(timestamped_edges.back()).target.vertex, 1, MPI_INT32_T, &status);
        MPI_File_read(input, &std::get<0>(timestamped_edges.back()).target.weight, 1, MPI_FLOAT, &status);
        MPI_File_read(input, &std::get<1>(timestamped_edges.back()), 1, MPI_INT32_T, &status);

        return true;
    };

    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);
    MPI_File input = gdsb::mpi::open_file(file_path);

    auto const [vertex_count, edge_count] = gdsb::mpi::read_binary_graph(input, std::move(read_f));
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 6);
}
