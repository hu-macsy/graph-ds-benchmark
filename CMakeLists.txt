# Set minimum version of CMake.
cmake_minimum_required(VERSION 3.11.4)

# Set project name and version
project(gdsb VERSION 0.2.0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(gdsb STATIC)

set(public_headers
  include/gdsb/graph_io.h
  include/gdsb/timer.h
  include/gdsb/experiment.h
  include/gdsb/batcher.h
  include/gdsb/sort_permutation.h
  include/gdsb/graph.h
)

target_sources(gdsb
  PRIVATE
    src/timer.cpp
    src/graph_io.cpp
    src/graph.cpp
    src/experiment.cpp
)

set_property(TARGET gdsb PROPERTY POSITION_INDEPENDENT_CODE ON)

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
      "Could not find test graph directory.")
  endif()

  add_executable(gdsb_test
    test/batcher.cpp
    test/experiment_tests.cpp
    test/graph_io_tests.cpp
    test/graph_test.cpp
  )

  target_link_libraries(gdsb_test PRIVATE gdsb Catch2::Catch2WithMain)

  install(
    TARGETS gdsb_test
    RUNTIME DESTINATION bin
  )

  # Test setup. Use CTest + Catch2's CTest integration.
  include(CTest)
  include(${PROJECT_SOURCE_DIR}/test/lib/Catch2/extras/Catch.cmake)
  catch_discover_tests(gdsb_test)
endif()
