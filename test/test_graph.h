#pragma once

#include <string>

static std::string const graph_path =
#ifdef GDSB_TEST_GRAPH_DIR
    std::string(GDSB_TEST_GRAPH_DIR) + "/";
#else
    "test/graphs/";
#endif

static std::string undirected_unweighted_temporal_reptilia_tortoise{ "reptilia-tortoise-network-pv.edges" };
static std::string small_weighted_temporal_graph{ "small_weighted_temporal_graph.edges" };
static std::string small_weighted_temporal_graph_bin{ "small_weighted_temporal_graph.bin" };
static std::string unweighted_directed_graph_enzymes{ "ENZYMES_g1.edges" };
static std::string unweighted_directed_graph_enzymes_bin{ "ENZYMES_g1.bin" };
static std::string undirected_unweighted_soc_dolphins{ "soc-dolphins.mtx" };
static std::string undirected_weighted_aves_songbird_social{ "aves-songbird-social.edges" };
static std::string undirected_unweighted_loops_ia_southernwomen{ "ia-southernwomen.edges" };

constexpr uint32_t enzymes_g1_vertex_count = 38;
constexpr uint32_t enzymes_g1_edge_count = 168;

static std::string const test_file_path =
#ifdef GDSB_TEST_FILES_DIR
    std::string(GDSB_TEST_FILES_DIR) + "/";
#else
    "test/files/";
#endif

static std::string test_txt{ "test.txt" };