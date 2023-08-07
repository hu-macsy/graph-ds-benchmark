# Graph Data Structure Benchmark GDSB

This library offers various tools for graph data structure experiments.

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
mkdir gdsb
cd gdsb
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install ../../
ninja
ninja install
```

## Tests


You can build the tests by setting the CMake option `GDSB_TEST` to `On`.

```
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DGDSB_TEST=On -DCMAKE_INSTALL_PREFIX=../../install ../../
ninja install
ninja
```
