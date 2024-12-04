#include <catch2/catch_test_macros.hpp>

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "test_graph.h"

#include <gdsb/batcher.h>
#include <gdsb/mpi_error_handler.h>
#include <gdsb/mpi_graph_io.h>

#include <filesystem>

using namespace gdsb;
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
        CHECK_NOTHROW(mpi::FileWrapper(file_path));
    }

    SECTION("Does not throw using valid path opening binary file.")
    {
        std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes_bin);
        CHECK_NOTHROW(mpi::FileWrapper(file_path));
    }

    SECTION("Throws using invalid path.")
    {
        std::string invalid_path = "this/is/an/invalid/path.bin";
        std::filesystem::path file_path(invalid_path.c_str());
        CHECK_THROWS(mpi::FileWrapper(file_path));
    }

    SECTION("Open for writing with overwriting will not fail.")
    {
        std::filesystem::path file_path(test_file_path + test_txt);
        CHECK_NOTHROW(mpi::FileWrapper(file_path, true, 0, MPI_MODE_CREATE | MPI_MODE_WRONLY));
    }
}

TEST_CASE("MPI, Read Small Weighted Temporal Binary File Header Information")
{
    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);

    mpi::FileWrapper input{ file_path };

    BinaryGraphHeader data = mpi::read_binary_graph_header(input.get());

    CHECK(data.directed == true);
    CHECK(data.weighted == true);
    CHECK(data.dynamic == true);
    CHECK(data.vertex_count == 7);
    CHECK(data.edge_count == 7);
}

TEST_CASE("MPI, Read Small Weighted Temporal Binary File")
{
    WeightedTimestampedEdges32 timestamped_edges;
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
    mpi::FileWrapper input{ file_path };

    auto const [vertex_count, edge_count] = mpi::read_binary_graph(input.get(), std::move(read_f));
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 7);

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
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 8);

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
    Edges32 edges;
    auto read_f = [&](MPI_File binary_graph)
    {
        edges.push_back(Edge32{});

        MPI_Status status;
        MPI_File_read(binary_graph, &edges.back().source, 1, MPI_INT32_T, &status);
        MPI_File_read(binary_graph, &edges.back().target, 1, MPI_INT32_T, &status);

        return true;
    };

    std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes_bin);
    mpi::FileWrapper binary_graph{ file_path };

    auto const [vertex_count, edge_count] = mpi::read_binary_graph(binary_graph.get(), std::move(read_f));

    CHECK(vertex_count == 38);
    CHECK(edge_count == 168);
    REQUIRE(edges.size() == edge_count);

    // TODO: The counterpart of reading this graph file using regular C++ file
    // I/O routines is capable to read the EOF symbol at the end of the file.
    // With MPI I/O however, the last read of the EOF symbol gives MPI_ERR_COUNT
    // (value: 2) as the error value of the MPI_Status.MPI_ERROR. It seems like
    // with MPI I/O we can't explicitly read the EOF symbol, or is there a way?
    //
    // The following code shows what is not working:
    // uint32_t eof_marker;
    // MPI_Status read_eof_status;
    // MPI_File_read(binary_graph, &eof_marker, 1, MPI_INT32_T, &read_eof_status);
    // REQUIRE(read_eof_status.MPI_ERROR == MPI_SUCCESS);

    bool edge_25_to_2_exists = std::any_of(std::begin(edges), std::end(edges),
                                           [](Edge32 const& edge) { return edge.source == 25 && edge.target == 2; });
    CHECK(edge_25_to_2_exists);

    bool edge_source_0_does_not_exist =
        std::none_of(std::begin(edges), std::end(edges), [](Edge32 const& edge) { return edge.source == 0; });
    CHECK(edge_source_0_does_not_exist);
}

TEST_CASE("MPI, read_binary_graph_partition, small weighted temporal, partition id 0, partition size 2")
{
    WeightedTimestampedEdges32 timestamped_edges;
    auto read_f = [&](MPI_File binary_graph)
    {
        timestamped_edges.push_back({});

        MPI_Status status;
        MPI_File_read(binary_graph, &timestamped_edges.back().edge.source, 1, MPI_INT32_T, &status);
        MPI_File_read(binary_graph, &timestamped_edges.back().edge.target.vertex, 1, MPI_INT32_T, &status);
        MPI_File_read(binary_graph, &timestamped_edges.back().edge.target.weight, 1, MPI_FLOAT, &status);
        MPI_File_read(binary_graph, &timestamped_edges.back().timestamp, 1, MPI_INT32_T, &status);

        return true;
    };

    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);
    mpi::FileWrapper binary_graph{ file_path };

    BinaryGraphHeader header = mpi::read_binary_graph_header(binary_graph.get());
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    uint32_t partition_id = 0;
    uint32_t partition_size = 2;
    auto const [vertex_count, edge_count] =
        mpi::read_binary_graph_partition(binary_graph.get(), header, std::move(read_f),
                                         sizeof(TimestampedEdge32), partition_id, partition_size);
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 3);

    // CHECK(!binary_graph.eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 1 4 1 8
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
    REQUIRE(timestamped_edges.size() == idx);
}

TEST_CASE("MPI, register structs")
{
    SECTION("register_weighted_timestamped_edge_32") { CHECK_NOTHROW(mpi::MPIWeightedTimestampedEdge32()); }

    SECTION("register_weighted_edge_32") { CHECK_NOTHROW(mpi::MPIWeightedEdge32()); }

    SECTION("register_edge_32") { CHECK_NOTHROW(mpi::MPIEdge32()); }
}

TEST_CASE("MPI, handle_type_create_struct_error, throws when expected")
{
    SECTION("Throws not on MPI_SUCCESS") { CHECK_NOTHROW(mpi::handle_type_create_struct_error(MPI_SUCCESS)); }

    SECTION("Throws on MPI_ERR_ARG, MPI_ERR_COUNT, MPI_ERR_TYPE, MPI_ERR_OTHER")
    {
        CHECK_THROWS(mpi::handle_type_create_struct_error(MPI_ERR_ARG));
        CHECK_THROWS(mpi::handle_type_create_struct_error(MPI_ERR_COUNT));
        CHECK_THROWS(mpi::handle_type_create_struct_error(MPI_ERR_TYPE));
        CHECK_THROWS(mpi::handle_type_create_struct_error(MPI_ERR_OTHER));
    }
}

TEST_CASE("MPI, FileWrapper, throws exception when path does not exist")
{
    std::filesystem::path filepath(graph_path + "this_path_does_not_exist.bin");

    SECTION("regular") { CHECK_THROWS(mpi::FileWrapper(filepath)); }
}

TEST_CASE("MPI, FileWrapper, throws no exception when file does already exist")
{
    std::filesystem::path filepath(graph_path + "this_path_does_not_exist_yet.bin");
    std::ofstream file(filepath, std::ios::out | std::ios::binary);

    char one_byte = 1;
    file.write(&one_byte, 1);

    CHECK_NOTHROW(mpi::FileWrapper(filepath, true));
}

TEST_CASE("MPI, all_read_binary_graph_partition, throws exception if file seek not successful")
{
    // The following copies the mpi::FileWrapper object when returning it by
    // value. Therefore, the destructor will be called on the old (copied)
    // object. Thus the file is closed. This then will lead to an issue when
    // trying to read from it later on throwing a runtime error.
    auto [binary_graph, header] = []()
    {
        std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);
        mpi::FileWrapper binary_graph{ file_path };

        BinaryGraphHeader header = mpi::read_binary_graph_header(binary_graph.get());

        return std::make_tuple(binary_graph, header);
    }();

    mpi::MPIWeightedTimestampedEdge32 mpi_timestamped_edge_t;

    uint32_t partition_id = 0;
    uint32_t partition_size = 2;
    WeightedTimestampedEdges32 timestamped_edges(partition_batch_count(header.edge_count, partition_id, partition_size));
    CHECK_THROWS_AS(mpi::all_read_binary_graph_partition(binary_graph.get(), header, &(timestamped_edges[0]),
                                                         sizeof(WeightedTimestampedEdge32),
                                                         mpi_timestamped_edge_t.get(), partition_id, partition_size),
                    std::runtime_error);
}

TEST_CASE("MPI, all_read_binary_graph_partition, small weighted temporal, partition id 0, partition size 2")
{
    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);
    mpi::FileWrapper binary_graph{ file_path };

    BinaryGraphHeader header = mpi::read_binary_graph_header(binary_graph.get());
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    mpi::MPIWeightedTimestampedEdge32 mpi_timestamped_edge_t;

    uint32_t partition_id = 0;
    uint32_t partition_size = 2;
    WeightedTimestampedEdges32 timestamped_edges(partition_batch_count(header.edge_count, partition_id, partition_size));
    auto const [vertex_count, edge_count] =
        mpi::all_read_binary_graph_partition(binary_graph.get(), header, &(timestamped_edges[0]), sizeof(WeightedTimestampedEdge32),
                                             mpi_timestamped_edge_t.get(), partition_id, partition_size);
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 3);

    // CHECK(!binary_graph.eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 1 4 1 8
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
    REQUIRE(timestamped_edges.size() == idx);
}

TEST_CASE("MPI, all_read_binary_graph_partition, small weighted temporal, partition id 1, partition size 2")
{
    std::filesystem::path file_path(graph_path + small_weighted_temporal_graph_bin);
    mpi::FileWrapper binary_graph{ file_path };

    BinaryGraphHeader header = mpi::read_binary_graph_header(binary_graph.get());
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.weight_byte_size == sizeof(Weight));

    mpi::MPIWeightedTimestampedEdge32 mpi_timestamped_edge_t;

    uint32_t partition_id = 1;
    uint32_t partition_size = 2;
    WeightedTimestampedEdges32 timestamped_edges(partition_batch_count(header.edge_count, partition_id, partition_size));
    auto const [vertex_count, edge_count] =
        mpi::all_read_binary_graph_partition(binary_graph.get(), header, &(timestamped_edges[0]), sizeof(WeightedTimestampedEdge32),
                                             mpi_timestamped_edge_t.get(), partition_id, partition_size);
    REQUIRE(vertex_count == 7);
    REQUIRE(edge_count == 4);

    // CHECK(!binary_graph.eof());

    // File content:
    // 0 1 1 1
    // 2 3 1 3
    // 1 2 1 2
    // 1 4 1 8
    // 3 4 1 4
    // 3 5 1 6
    // 3 6 1 7

    size_t idx = 0;
    REQUIRE(timestamped_edges.size() == edge_count);
    CHECK(timestamped_edges[idx].edge.source == 1);
    CHECK(timestamped_edges[idx].edge.target.vertex == 4);
    CHECK(timestamped_edges[idx].edge.target.weight == 1.f);
    CHECK(timestamped_edges[idx].timestamp == 8);

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

TEST_CASE("MPI, all_read_binary_graph_partition, undirected, unweighted, static, partition id 0, partition size 4")
{
    std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes_bin);
    mpi::FileWrapper binary_graph{ file_path };

    BinaryGraphHeader header = mpi::read_binary_graph_header(binary_graph.get());
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.directed);
    REQUIRE(!header.weighted);
    REQUIRE(!header.dynamic);

    mpi::MPIEdge32 mpi_edge_t;

    uint32_t partition_id = 0;
    uint32_t partition_size = 4;
    Edges32 edges(partition_batch_count(header.edge_count, partition_id, partition_size));
    auto const [vertex_count, edge_count] =
        mpi::all_read_binary_graph_partition(binary_graph.get(), header, &(edges[0]), sizeof(Edge32), mpi_edge_t.get(),
                                             partition_id, partition_size);

    REQUIRE(vertex_count == enzymes_g1_vertex_count);
    REQUIRE(edge_count == 42);
    REQUIRE(edges.size() == edge_count);

    size_t idx = 0;
    CHECK((edges[idx].source == 2 && edges[idx++].target == 1));
    CHECK((edges[idx].source == 3 && edges[idx++].target == 1));
    CHECK((edges[idx].source == 4 && edges[idx++].target == 1));
    CHECK((edges[idx].source == 1 && edges[idx++].target == 2));
    CHECK((edges[idx].source == 3 && edges[idx++].target == 2));
    CHECK((edges[idx].source == 4 && edges[idx++].target == 2));
    CHECK((edges[idx].source == 25 && edges[idx++].target == 2));
    CHECK((edges[idx].source == 28 && edges[idx++].target == 2));
    CHECK((edges[idx].source == 1 && edges[idx++].target == 3));
    CHECK((edges[idx].source == 2 && edges[idx++].target == 3));
    CHECK((edges[idx].source == 4 && edges[idx++].target == 3));
    CHECK((edges[idx].source == 28 && edges[idx++].target == 3));
    CHECK((edges[idx].source == 29 && edges[idx++].target == 3));
    CHECK((edges[idx].source == 1 && edges[idx++].target == 4));
    CHECK((edges[idx].source == 2 && edges[idx++].target == 4));
    CHECK((edges[idx].source == 3 && edges[idx++].target == 4));
    CHECK((edges[idx].source == 5 && edges[idx++].target == 4));
    CHECK((edges[idx].source == 6 && edges[idx++].target == 4));
    CHECK((edges[idx].source == 29 && edges[idx++].target == 4));
    CHECK((edges[idx].source == 4 && edges[idx++].target == 5));
    CHECK((edges[idx].source == 6 && edges[idx++].target == 5));
    CHECK((edges[idx].source == 7 && edges[idx++].target == 5));
    CHECK((edges[idx].source == 30 && edges[idx++].target == 5));
    CHECK((edges[idx].source == 4 && edges[idx++].target == 6));
    CHECK((edges[idx].source == 5 && edges[idx++].target == 6));
    CHECK((edges[idx].source == 7 && edges[idx++].target == 6));
    CHECK((edges[idx].source == 8 && edges[idx++].target == 6));
    CHECK((edges[idx].source == 30 && edges[idx++].target == 6));
    CHECK((edges[idx].source == 5 && edges[idx++].target == 7));
    CHECK((edges[idx].source == 6 && edges[idx++].target == 7));
    CHECK((edges[idx].source == 8 && edges[idx++].target == 7));
    CHECK((edges[idx].source == 9 && edges[idx++].target == 7));
    CHECK((edges[idx].source == 6 && edges[idx++].target == 8));
    CHECK((edges[idx].source == 7 && edges[idx++].target == 8));
    CHECK((edges[idx].source == 9 && edges[idx++].target == 8));
    CHECK((edges[idx].source == 10 && edges[idx++].target == 8));
    CHECK((edges[idx].source == 11 && edges[idx++].target == 8));
    CHECK((edges[idx].source == 7 && edges[idx++].target == 9));
    CHECK((edges[idx].source == 8 && edges[idx++].target == 9));
    CHECK((edges[idx].source == 10 && edges[idx++].target == 9));
    CHECK((edges[idx].source == 8 && edges[idx++].target == 10));
    CHECK((edges[idx].source == 9 && edges[idx++].target == 10));

    CHECK(idx == edges.size());
}

TEST_CASE("MPI, all_read_binary_graph_partition, undirected, unweighted, static, partition id 0, partition size 1")
{
    std::filesystem::path file_path(graph_path + unweighted_directed_graph_enzymes_bin);
    mpi::FileWrapper binary_graph{ file_path };

    BinaryGraphHeader header = mpi::read_binary_graph_header(binary_graph.get());
    REQUIRE(header.vertex_id_byte_size == sizeof(Vertex32));
    REQUIRE(header.directed);
    REQUIRE(!header.weighted);
    REQUIRE(!header.dynamic);

    mpi::MPIEdge32 mpi_edge_t;

    uint32_t partition_id = 0;
    uint32_t partition_size = 1;
    Edges32 edges(partition_batch_count(header.edge_count, partition_id, partition_size));
    auto const [vertex_count, edge_count] =
        mpi::all_read_binary_graph_partition(binary_graph.get(), header, &(edges[0]), sizeof(Edge32), mpi_edge_t.get(),
                                             partition_id, partition_size);

    REQUIRE(vertex_count == enzymes_g1_vertex_count);
    REQUIRE(edge_count == enzymes_g1_edge_count);
    REQUIRE(edges.size() == edge_count);
}