cmake_minimum_required(VERSION 3.16)
project(wht_audio CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_FFTW "Enable FFTW backend for STFT" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

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
    pkg_check_modules(FFTW3F REQUIRED fftw3f)
    target_compile_definitions(core PRIVATE USE_FFTW=1)
    target_include_directories(core PRIVATE ${FFTW3F_INCLUDE_DIRS})
    target_link_libraries(core PRIVATE ${FFTW3F_LIBRARIES})
endif()

add_executable(app
    apps/main.cpp
)
target_link_libraries(app PRIVATE core)

add_executable(test_wht
    tests/test_wht.cpp
)
target_link_libraries(test_wht PRIVATE core)

add_executable(test_metrics
    tests/test_metrics.cpp
)
target_link_libraries(test_metrics PRIVATE core)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic -O3)
    target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic -O3)
    target_compile_options(test_wht PRIVATE -Wall -Wextra -Wpedantic -O3)
    target_compile_options(test_metrics PRIVATE -Wall -Wextra -Wpedantic -O3)
endif()