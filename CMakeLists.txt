# Set minimum version of CMake.
cmake_minimum_required(VERSION 3.11.4)

# Set project name and version
project(gdsb VERSION 0.2.0)

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

set_property(TARGET gdsb PROPERTY POSITION_INDEPENDENT_CODE ON)

# Set this option if you want to use the MPI facilities of GDSB 
option(GDSB_MPI "Build GDSB MPI library.")

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
    test/batcher.cpp
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


