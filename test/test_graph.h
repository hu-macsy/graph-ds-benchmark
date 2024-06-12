#pragma once

#include <string>

// https://networkrepository.com/bio-celegans.php
static std::string const graph_path =
#ifdef GDSB_TEST_GRAPH_DIR
    std::string(GDSB_TEST_GRAPH_DIR) + "/";
#else
    "test/graphs/";
#endif

static std::string undirected_unweighted_temporal_reptilia_tortoise{ "reptilia-tortoise-network-pv.edges" };
static std::string small_weighted_temporal_graph{ "small_graph_temporal.edges" };
static std::string small_weighted_temporal_graph_bin{ "small_graph_temporal.bin" };
static std::string unweighted_directed_graph_enzymes{ "ENZYMES_g1.edges" };
static std::string unweighted_directed_graph_enzymes_bin{ "ENZYMES_g1.bin" };
static std::string undirected_unweighted_soc_dolphins{ "soc-dolphins.mtx" };

constexpr uint32_t enzymes_g1_vertex_count = 38;
constexpr uint32_t enzymes_g1_edge_count = 168;