# Set minimum version of CMake.
cmake_minimum_required(VERSION 3.14)

# Set project name and version
project(gdsb VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(gdsb STATIC)

set(public_headers
  include/gdsb/batcher.h
  include/gdsb/experiment.h
  include/gdsb/graph_input.h
  include/gdsb/graph_io_parameters.h
  include/gdsb/graph_output.h
  include/gdsb/graph.h
  include/gdsb/sort_permutation.h
  include/gdsb/timer.h
)

target_sources(gdsb
  PRIVATE
    src/timer.cpp
    src/graph_input.cpp
    src/graph.cpp
    src/experiment.cpp
)

find_package(OpenMP)

# Thanks to NetworKit for the following OpenMP code looking or OpenMP in the
# most common folders:
# https://github.com/networkit/networkit/blob/master/CMakeLists.txt#L145
if(NOT OpenMP_FOUND)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # This will find default libomp-installations for homebrew/MacPorts
    find_library(LIBOMP_PATH NAMES omp PATHS "/usr/local/opt/libomp/lib" "/opt/local/lib/libomp" "/opt/homebrew/opt/libomp/lib")
    find_path(LIBOMP_INCLUDE NAMES omp.h PATHS "/usr/local/opt/libomp/include" "/opt/local/include/libomp" "/opt/homebrew/opt/libomp/include")

    if(LIBOMP_PATH AND LIBOMP_INCLUDE)
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${LIBOMP_INCLUDE}" CACHE STRING "Manually set" FORCE)
    endif()
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(DEFINED ENV{CONDA_PREFIX})
      find_library(LIBOMP_PATH NAMES omp HINTS "$ENV{CONDA_PREFIX}/lib"
        PATHS "/usr/lib" "/usr/lib64")
      find_path(LIBOMP_INCLUDE NAMES omp.h HINTS "$ENV{CONDA_PREFIX}/include"
        PATHS "/usr/include")
    else()
      find_library(LIBOMP_PATH NAMES omp PATHS "/usr/lib" "/usr/lib64")
      find_path(LIBOMP_INCLUDE NAMES omp.h PATHS "/usr/include")
    endif()

    if(LIBOMP_PATH AND LIBOMP_INCLUDE)
      set(OpenMP_CXX_FLAGS "-fopenmp -I${LIBOMP_INCLUDE}" CACHE STRING "Manually set" FORCE)
    endif()
  endif()

  # Set OpenMP-folders in case they are found with the aid of additional hints
  if(LIBOMP_PATH AND LIBOMP_INCLUDE)
    set(OpenMP_CXX_LIB_NAMES "omp" CACHE STRING "Manually set" FORCE)
    set(OpenMP_omp_LIBRARY "${LIBOMP_PATH}" CACHE STRING "Manually set" FORCE)
  else()
    message(FATAL_ERROR "libomp was not found, but necessary to configure DHB with ${CMAKE_CXX_COMPILER_ID}")
  endif()

  # After setting basic OpenMP-folders, run find_package again to set everything. Also acts as a final sanity check.
  find_package(OpenMP REQUIRED)
endif()

target_link_libraries(gdsb PUBLIC OpenMP::OpenMP_CXX)

set_property(TARGET gdsb PROPERTY POSITION_INDEPENDENT_CODE ON)

# Set this option if you want to use the MPI facilities of GDSB 
option(GDSB_MPI "Build GDSB with MPI functionality." OFF)

if (GDSB_MPI)
  set(public_headers
    ${public_headers}
    include/gdsb/mpi_graph_io.h
    include/gdsb/mpi_error_handler.h
  )

  target_sources(gdsb
    PRIVATE
      src/mpi_error_handler.cpp
      src/mpi_graph_io.cpp
  )

  find_package(MPI REQUIRED)
  target_link_libraries(gdsb PUBLIC MPI::MPI_CXX)
endif()

target_include_directories(gdsb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS gdsb
    EXPORT "gdsbTargets"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${public_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdsb)

install(EXPORT "gdsbTargets"
  FILE "gdsbTargets.cmake"
  NAMESPACE gdsb::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gdsb
)

add_library(gdsb::gdsb STATIC IMPORTED)

set_target_properties(gdsb::gdsb PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)

set_target_properties(gdsb PROPERTIES PUBLIC_HEADER "${public_headers}")
set_target_properties(gdsb PROPERTIES DEBUG_POSTFIX "d")

# ===================================
# Make a Configuration Package
# ===================================
include(CMakePackageConfigHelpers)

export(EXPORT gdsbTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/gdsbTargets.cmake"
       NAMESPACE gdsb::
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/gdsbConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gdsb
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gdsbConfigVersion.cmake"
  VERSION "${version}"
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/gdsbConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/gdsbConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gdsb
)

set_property(TARGET gdsb PROPERTY VERSION ${version})

set_target_properties(gdsb PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

option(GDSB_TEST "Build test target." OFF)
if (GDSB_TEST)
  if(EXISTS "${PROJECT_SOURCE_DIR}/test/lib/Catch2")
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/lib/Catch2)
  else()
    message(FATAL_ERROR
        "Missing Catch2 library in test/lib/Catch2 "
        "Please run `git submodule update --init` to fetch the submodule.")
  endif()

  if(EXISTS "${PROJECT_SOURCE_DIR}/test/graphs")
    add_definitions(-DGDSB_TEST_GRAPH_DIR="${PROJECT_SOURCE_DIR}/test/graphs")
  else()
    message(FATAL_ERROR
      "Could not find test graphs directory.")
  endif()

  if(EXISTS "${PROJECT_SOURCE_DIR}/test/files")
    add_definitions(-DGDSB_TEST_FILES_DIR="${PROJECT_SOURCE_DIR}/test/files")
  else()
    message(FATAL_ERROR
      "Could not find test files directory.")
  endif()

  # GDSB test target
  add_executable(gdsb_test
    test/batcher_tests.cpp
    test/experiment_tests.cpp
    test/graph_input_tests.cpp
    test/graph_test.cpp
    test/graph_output_tests.cpp
  )

  # Debugging Libraries
  # target_link_libraries(gdsb_test PRIVATE asan)

  target_link_libraries(gdsb_test PRIVATE gdsb Catch2::Catch2WithMain)
  
  install(
    TARGETS gdsb_test
    RUNTIME DESTINATION bin
  )

  # GDSB MPI test target
  if (GDSB_MPI)
    add_executable(gdsb_mpi_test
      test/mpi_graph_io_tests.cpp
    )

    # Debugging Libraries
    # target_link_libraries(gdsb_mpi_test PRIVATE asan)

    target_link_libraries(gdsb_mpi_test PRIVATE gdsb Catch2::Catch2WithMain MPI::MPI_CXX)

    install(
      TARGETS gdsb_mpi_test
      RUNTIME DESTINATION bin
    )
  endif()

  # Test setup. Use CTest + Catch2's CTest integration.
  include(CTest)
  include(${PROJECT_SOURCE_DIR}/test/lib/Catch2/extras/Catch.cmake)
  catch_discover_tests(gdsb_test)
endif()


