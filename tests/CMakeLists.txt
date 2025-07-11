# Copyright (c) 2014-2024, The Prolo Project
# 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other
#    materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors may be
#    used to endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

set(PROLO_WALLET_CRYPTO_BENCH "auto" CACHE STRING "Select wallet crypto libraries for benchmarking")

# The docs say this only affects grouping in IDEs
set(folder "tests")
set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")

if (WIN32 AND STATIC)
  add_definitions(-DSTATICLIB)
  # miniupnp changed their static define
  add_definitions(-DMINIUPNP_STATICLIB)
endif ()

set(BUILD_GMOCK OFF CACHE BOOL "Don't build GMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Don't install GTest" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/gtest" ${CMAKE_BINARY_DIR}/googletest)
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/external/gtest/googletest/include")
set(GTEST_LIBRARIES gtest gtest_main)

message(STATUS "Copying test data directory...")
file(COPY data DESTINATION .) # Copy data directory from source root to build root

if (CMAKE_BUILD_TYPE STREQUAL "fuzz" OR OSSFUZZ)
  add_subdirectory(fuzz)
else ()
  add_subdirectory(core_tests)
  add_subdirectory(fuzz)
  add_subdirectory(crypto)
  add_subdirectory(functional_tests)
  add_subdirectory(performance_tests)
  add_subdirectory(unit_tests)
  add_subdirectory(difficulty)
  add_subdirectory(block_weight)
  add_subdirectory(hash)
  add_subdirectory(net_load_tests)
endif()

if (BUILD_GUI_DEPS)
  add_subdirectory(libwallet_api_tests)
endif()

if (TREZOR_DEBUG)
  add_subdirectory(trezor)
endif()

set(hash_targets_sources
  hash-target.cpp)

set(hash_targets_headers)

prolo_add_minimal_executable(hash-target-tests
  ${hash_targets_sources}
  ${hash_targets_headers})
target_link_libraries(hash-target-tests
  PRIVATE
    cryptonote_core)
set_property(TARGET hash-target-tests
  PROPERTY
    FOLDER "${folder}")

add_test(
  NAME    hash-target
  COMMAND hash-target-tests)

#
# Configure wallet crypto benchmark
#
if (${PROLO_WALLET_CRYPTO_BENCH} STREQUAL "auto")
  set(PROLO_WALLET_CRYPTO_BENCH "cn")
  prolo_crypto_autodetect(AVAILABLE BEST)
  if (DEFINED AVAILABLE)
    list(APPEND PROLO_WALLET_CRYPTO_BENCH ${AVAILABLE})
  endif ()
  message("Wallet crypto bench is using ${PROLO_WALLET_CRYPTO_BENCH}")
endif ()

list(REMOVE_DUPLICATES PROLO_WALLET_CRYPTO_BENCH)
list(REMOVE_ITEM PROLO_WALLET_CRYPTO_BENCH "cn") # always used for comparison
set(PROLO_WALLET_CRYPTO_BENCH_NAMES "(cn)")
foreach(BENCH IN LISTS PROLO_WALLET_CRYPTO_BENCH)
  prolo_crypto_valid(${BENCH} VALID)
  if (NOT VALID)
    message(FATAL_ERROR "Invalid PROLO_WALLET_CRYPTO_BENCH option ${BENCH}")
  endif ()

  prolo_crypto_get_target(${BENCH} BENCH_LIBRARY)
  list(APPEND BENCH_OBJECTS $<TARGET_OBJECTS:${BENCH_LIBRARY}>)

  prolo_crypto_get_namespace(${BENCH} BENCH_NAMESPACE)
  set(PROLO_WALLET_CRYPTO_BENCH_NAMES "${PROLO_WALLET_CRYPTO_BENCH_NAMES}(${BENCH_NAMESPACE})")
endforeach ()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/benchmark.h.in" "${PROLO_GENERATED_HEADERS_DIR}/tests/benchmark.h")
prolo_add_minimal_executable(prolo-wallet-crypto-bench benchmark.cpp ${BENCH_OBJECTS})
target_link_libraries(prolo-wallet-crypto-bench cncrypto)

add_test(NAME wallet-crypto-bench COMMAND prolo-wallet-crypto-bench)


set(enabled_tests
    core_tests
    difficulty
    block_weight
    hash
    performance_tests
    fuzz
    unit_tests)

add_custom_target(tests DEPENDS enabled_tests)
set_property(TARGET tests PROPERTY FOLDER "${folder}")
