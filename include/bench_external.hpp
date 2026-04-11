#pragma once

#include <cstddef>
#include <vector>

namespace bench_external {

double run_ffht_single_frame_ms(const std::vector<float>& input, std::size_t iterations);

double run_fftw_r2c_single_frame_ms(const std::vector<float>& input, std::size_t iterations);

}