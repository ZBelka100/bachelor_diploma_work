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

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

if (USE_FFTW)
    pkg_check_modules(FFTW3F REQUIRED fftw3f)
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

target_include_directories(core PUBLIC
    include
    ${SNDFILE_INCLUDE_DIRS}
)

target_link_libraries(core PUBLIC
    ${SNDFILE_LIBRARIES}
)

target_compile_options(core PRIVATE
    ${SNDFILE_CFLAGS_OTHER}
)

if (USE_FFTW)
    target_compile_definitions(core PRIVATE USE_FFTW=1)
    target_include_directories(core PRIVATE ${FFTW3F_INCLUDE_DIRS})
    target_link_libraries(core PRIVATE ${FFTW3F_LIBRARIES})
endif()

add_library(bench_external
    src/bench_external.cpp
)

target_include_directories(bench_external PUBLIC
    include
)

if (USE_FFHT)
    target_include_directories(bench_external PRIVATE
        third_party/ffht
    )
    target_compile_definitions(bench_external PRIVATE USE_FFHT=1)
endif()

if (USE_FFTW)
    target_include_directories(bench_external PRIVATE ${FFTW3F_INCLUDE_DIRS})
    target_link_libraries(bench_external PRIVATE ${FFTW3F_LIBRARIES})
    target_compile_definitions(bench_external PRIVATE USE_FFTW=1)
endif()

add_executable(app
    apps/main.cpp
)
target_link_libraries(app PRIVATE core)

add_executable(benchmark_app
    apps/benchmark.cpp
)
target_link_libraries(benchmark_app PRIVATE core bench_external)

add_executable(test_wht
    tests/test_wht.cpp
)
target_link_libraries(test_wht PRIVATE core)

add_executable(test_metrics
    tests/test_metrics.cpp
)
target_link_libraries(test_metrics PRIVATE core)

function(enable_strict_warnings target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()
endfunction()

function(enable_perf_opts target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE
            -O3
            -DNDEBUG
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
enable_strict_warnings(test_wht)
enable_strict_warnings(test_metrics)

enable_perf_opts(core)
enable_perf_opts(bench_external)
enable_perf_opts(app)
enable_perf_opts(benchmark_app)
enable_perf_opts(test_wht)
enable_perf_opts(test_metrics)