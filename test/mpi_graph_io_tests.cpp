#include <catch2/catch_test_macros.hpp>

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "test_graph.h"

#include <gdsb/mpi_error_handler.h>
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

TEST_CASE("MPI, Open File")
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

TEST_CASE("MPI, Read Small Weighted Temporal Binary File Header Information")
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

TEST_CASE("MPI, Read Small Weighted Temporal Binary File")
{
    gdsb::TimestampedEdges32 timestamped_edges;
    auto read_f = [&](MPI_File input)
    {
        timestamped_edges.push_back({});

        MPI_Status status;
        MPI_File_read(input, &timestamped_edges.back().edge.source, 1, MPI_INT32_T, &status);
        MPI_File_read(input, &timestamped_edges.back().edge.target.vertex, 1, MPI_INT32_T, &status);
        MPI_File_read(input, &timestamped_edges.back().edge.target.weight, 1, MPI_FLOAT, &status);
        MPI_File_read(input, &timestamped_edges.back().timestamp, 1, MPI_INT32_T, &status);

        return true;
    };

    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);
    MPI_File input = gdsb::mpi::open_file(file_path);

    auto const [vertex_count, edge_count] = gdsb::mpi::read_binary_graph(input, std::move(read_f));
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 6);

    // Note: EOF is an int (for most OS?)
    // int eof_marker;
    // MPI_Status status;
    // MPI_File_read(input, &eof_marker, 1, MPI_INT32_T, &status);
    // REQUIRE(eof_marker == 10);
    // Note:
    // The EOF representation in the file evaluates to 10 interpreted as int. In
    // many cases it actually is represented as a -1:
    // https://en.cppreference.com/w/cpp/string/char_traits/eof
    // The following test fails though:
    // REQUIRE(eof_marker == std::char_traits<char>::eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 3 4 1 4
    // 3 5 1 6
    // 3 6 1 7

    size_t idx = 0;
    REQUIRE(timestamped_edges.size() == edge_count);
    CHECK(timestamped_edges[idx].edge.source == 0);
    CHECK(timestamped_edges[idx].edge.target.vertex == 1);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 1);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 2);
    CHECK(timestamped_edges[idx].edge.target.vertex == 3);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 3);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 2);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 2);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 4);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 5);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 6);

    ++idx;
    CHECK(timestamped_edges[idx].edge.source == 3);
    CHECK(timestamped_edges[idx].edge.target.vertex == 6);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 7);

    ++idx;
    REQUIRE(timestamped_edges.size() == idx);
}


TEST_CASE("MPI, read_binary_graph, undirected, unweighted, static")
{
    gdsb::Edges32 edges;
    auto read_f = [&](MPI_File input)
    {
        edges.push_back(gdsb::Edge32{});

        MPI_Status status;
        MPI_File_read(input, &edges.back().source, 1, MPI_INT32_T, &status);
        MPI_File_read(input, &edges.back().target.vertex, 1, MPI_INT32_T, &status);

        return true;
    };

    std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes_bin);
    MPI_File input = gdsb::mpi::open_file(file_path);

    auto const [vertex_count, edge_count] = gdsb::mpi::read_binary_graph(input, std::move(read_f));

    // TODO: check for EOF!

    CHECK(vertex_count == 38);
    CHECK(edge_count == 168);
    REQUIRE(edges.size() == edge_count);

    std::any_of(std::begin(edges), std::end(edges),
                [](gdsb::Edge32 edge)
                { return edge.source == 25 && edge.target.vertex == 2 && edge.target.weight == 1.f; });
    std::none_of(std::begin(edges), std::end(edges), [](gdsb::Edge32 edge) { return edge.source == 1; });
}

TEST_CASE("MPI, handle_type_create_struct_error, throws when expected")
{
    SECTION("Throws not on MPI_SUCCESS") { CHECK_NOTHROW(gdsb::mpi::handle_type_create_struct_error(MPI_SUCCESS)); }

    SECTION("Throws on MPI_ERR_ARG, MPI_ERR_COUNT, MPI_ERR_TYPE, MPI_ERR_OTHER")
    {
        CHECK_THROWS(gdsb::mpi::handle_type_create_struct_error(MPI_ERR_ARG));
        CHECK_THROWS(gdsb::mpi::handle_type_create_struct_error(MPI_ERR_COUNT));
        CHECK_THROWS(gdsb::mpi::handle_type_create_struct_error(MPI_ERR_TYPE));
        CHECK_THROWS(gdsb::mpi::handle_type_create_struct_error(MPI_ERR_OTHER));
    }
}