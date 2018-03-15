# Copyright 2016 Google Inc.  All Rights Reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# Old enough to support Ubuntu Precise.
cmake_minimum_required(VERSION 2.8.7)

if (WIN32)
  if (NOT EXISTS "compact_enc_det/compact_enc_det.h")
    message(FATAL_ERROR "\nCould not find source code.  Make sure you are running this script from the root of the distribution tree.")
  endif()

  if (NOT EXISTS "gtest")
    message(STATUS "Google Test not present.  Fetching from the web...")
    file(DOWNLOAD "https://github.com/google/googletest/archive/master.zip" ${CMAKE_SOURCE_DIR}/master.zip)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar x master.zip)
    file(REMOVE master.zip)
    file(RENAME googletest-master gtest)
  endif()

  # Configure gtest.
  execute_process(COMMAND ${CMAKE_COMMAND} .
                          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gtest)
endif()

project(CED CXX)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-narrowing")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-c++11-narrowing")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if (NOT BUILD_SHARED_LIBS)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
  endif()
endif()

set(EXTRA_TARGET_LINK_LIBRARIES)

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE -DSTRICT -DNOMINMAX)
  set(THREADING threadwin)
else()
  set(THREADING thread)
  list(APPEND EXTRA_TARGET_LINK_LIBRARIES -pthread)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR})

set(CED_LIBRARY_SOURCES
    compact_enc_det/compact_enc_det.cc
    compact_enc_det/compact_enc_det_hint_code.cc
    util/encodings/encodings.cc
    util/languages/languages.cc
    )

add_library(ced ${CED_LIBRARY_SOURCES})

# Uncomment to put CED into WHATWG-compliant mode.
#add_definitions(-DHTML5_MODE)

set(GTEST_INCLUDE_DIR "gtest/googletest/include")
set(GTEST_LIB_DIR "${CMAKE_SOURCE_DIR}/gtest/googlemock/gtest")

set(CED_UNITTEST_SOURCES
    compact_enc_det/compact_enc_det_unittest.cc
    compact_enc_det/compact_enc_det_fuzz_test.cc
    compact_enc_det/detail_head_string.inc
    util/encodings/encodings_unittest.cc
    )

add_executable(ced_unittest ${CED_UNITTEST_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/${GTEST_INCLUDE_DIR})
if (WIN32)
  set(GTEST_LIB_DIR_DBG "${GTEST_LIB_DIR}/Debug")
  set(GTEST_LIB_DIR_REL "${GTEST_LIB_DIR}/Release")
  target_link_libraries(ced_unittest
                        ced
                        debug ${GTEST_LIB_DIR_DBG}/gtest.lib optimized ${GTEST_LIB_DIR_REL}/gtest.lib
                        debug ${GTEST_LIB_DIR_DBG}/gtest_main.lib optimized ${GTEST_LIB_DIR_REL}/gtest_main.lib
                        ${EXTRA_TARGET_LINK_LIBRARIES})
  message("\nConfiguration completed.  Open the created projects with Visual Studio to build the packages.\n")
else()
  target_link_libraries(ced_unittest
                        ced
                        ${GTEST_LIB_DIR}/libgtest.a
                        ${GTEST_LIB_DIR}/libgtest_main.a
                        ${EXTRA_TARGET_LINK_LIBRARIES}
                        )
endif()
