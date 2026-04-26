cmake_minimum_required(VERSION 3.16)
project(wht_audio CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(USE_FFTW "Enable FFTW backend for STFT and benchmarks" ON)
option(USE_FFHT "Enable external FFHT benchmark" ON)
option(ENABLE_FAST_MATH "Enable fast math optimizations" OFF)
option(ENABLE_NATIVE_OPT "Enable -march=native" ON)
option(BUILD_TESTING "Build and enable tests" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)

if (USE_FFTW)
    pkg_check_modules(FFTW3F REQUIRED IMPORTED_TARGET fftw3f)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

add_library(core
    src/audio_io.cpp
    src/wht.cpp
    src/window.cpp
    src/framing.cpp
    src/pseudospectrum.cpp
    src/reconstruction.cpp
    src/stft.cpp
    src/metrics.cpp
    src/export.cpp
)

target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC PkgConfig::SNDFILE)

if (USE_FFTW)
    target_compile_definitions(core PRIVATE USE_FFTW=1)
    target_link_libraries(core PUBLIC PkgConfig::FFTW3F)
endif()

add_library(bench_external src/bench_external.cpp)
target_include_directories(bench_external PUBLIC include)

if (USE_FFHT)
    target_include_directories(bench_external PRIVATE third_party/ffht)
    target_compile_definitions(bench_external PRIVATE USE_FFHT=1)
endif()

if (USE_FFTW)
    target_compile_definitions(bench_external PRIVATE USE_FFTW=1)
    target_link_libraries(bench_external PUBLIC PkgConfig::FFTW3F)
endif()

add_executable(app apps/main.cpp)
target_link_libraries(app PRIVATE core)

if (USE_FFTW)
    target_link_libraries(app PRIVATE PkgConfig::FFTW3F)
endif()

add_executable(benchmark_app apps/benchmark.cpp)
target_link_libraries(benchmark_app PRIVATE core bench_external)

if (USE_FFTW)
    target_link_libraries(benchmark_app PRIVATE PkgConfig::FFTW3F)
endif()

function(enable_strict_warnings target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE
            -Wall -Wextra -Wpedantic
        )
    endif()
endfunction()

function(enable_perf_opts target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE
            -O3 -DNDEBUG
        )

        if (ENABLE_NATIVE_OPT)
            target_compile_options(${target_name} PRIVATE -march=native)
        endif()

        if (ENABLE_FAST_MATH)
            target_compile_options(${target_name} PRIVATE -ffast-math)
        endif()
    endif()

    if (IPO_SUPPORTED)
        set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()

enable_strict_warnings(core)
enable_strict_warnings(bench_external)
enable_strict_warnings(app)
enable_strict_warnings(benchmark_app)

enable_perf_opts(core)
enable_perf_opts(bench_external)
enable_perf_opts(app)
enable_perf_opts(benchmark_app)

if (BUILD_TESTING)
    enable_testing()

    function(add_unit_test test_name)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE core)
        add_test(NAME ${test_name} COMMAND ${test_name})
        enable_strict_warnings(${test_name})
    endfunction()

    add_unit_test(test_wht)
    add_unit_test(test_metrics)
    add_unit_test(test_stft)
    add_unit_test(test_framing)
    add_unit_test(test_window)
    add_unit_test(test_pseudospectrum)
    add_unit_test(test_reconstruction)
    add_unit_test(test_audio_io)
    add_unit_test(test_export)
    add_unit_test(test_integration)
endif()