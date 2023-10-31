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
static std::string unweighted_directed_graph_enzymes{ "ENZYMES_g1.edges" };
static std::string undirected_unweighted_soc_dolphins{ "soc-dolphins.mtx" };