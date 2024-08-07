# Graph Data Structure Benchmark GDSB

<p align="center">
  <a href="https://github.com/hu-macsy/graph-ds-benchmark/actions/workflows/ci.yml"><img src="https://github.com/hu-macsy/graph-ds-benchmark/actions/workflows/ci.yml/badge.svg"></a>
</p>

This library offers various tools for graph data structure experiments including:
- graph file I/O, see [graph_io.h](/include/gdsb/graph_io.h)
- graph and edge data structures, see [graph.h](/include/gdsb/graph.h)
- experiment environment to benchmark procedures, see
  [experiment.h](/include/gdsb/experiment.h)
- time measurement facilities, see [timer.h](/include/gdsb/timer.h)
- sorting containers using a specific permutation, see
  [sort_permutation.h](/include/gdsb/sort_permutation.h)
- a batcher system to create and apply edge batches which may be fed to a graph
  data structure, see [sort_permutation.h](/include/gdsb/batcher.h)

## Releases

### Version 0.2.0

A complete revision of graph file I/O to read edges from a file reducing the
formerly defined functions to read in directed, undirected, weighted,
unweighted, dynamic, or static graphs to a single function. In addition, the
graph data structure primitives have been revised to reflect their 32 or 64 bit
nature as well as a few minor fixes. 

In short, this version addresses the complexity of file I/O options trying to
simplify the API.

| Version Number           | Tag       | URL                                                        |
| -------------------------|-----------|------------------------------------------------------------|
| `0.2.0`                  | `v0.2.0`  | https://github.com/hu-macsy/graph-ds-benchmark/tree/v0.2.0 |

### Version 0.0.1

This version includes timer, graph (I/O), and sorting functionality. Also
includes a batcher system and an experiment environment for benchmarking.

| Version Number        | Tag       | URL                                                        |
| ----------------------|-----------|------------------------------------------------------------|
| `0.0.1`               | `v0.0.1`  | https://github.com/hu-macsy/graph-ds-benchmark/tree/v0.0.1 |

## Authors

Sorted alphabetically.

| Name                      | E-Mail                                  | Affiliation |
|---------------------------|-----------------------------------------|-------------|
| Alexander van der Grinten | n/a                                     | n/a         |
| Florian Willich           | florian.willich@informatik.hu-berlin.de | HU Berlin   |

# Build 

Build the library using CMake, generator Ninja.

```bash
mkdir install
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install ../
ninja
ninja install
```

## Tests


You can build the tests by setting the CMake option `GDSB_TEST` to `On`.

```
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DGDSB_TEST=On -DCMAKE_INSTALL_PREFIX=../install ../
ninja install
ninja
```
