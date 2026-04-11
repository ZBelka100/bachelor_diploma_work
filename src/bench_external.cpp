#include "bench_external.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <vector>

#define FHT_HEADER_ONLY
#include "fht_header_only.h"

#include <fftw3.h>

namespace {
using Clock = std::chrono::steady_clock;

bool is_power_of_two(std::size_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

int ilog2_exact(std::size_t n) {
    int p = 0;
    while ((std::size_t(1) << p) < n) {
        ++p;
    }
    if ((std::size_t(1) << p) != n) {
        throw std::invalid_argument("Input length must be a power of two");
    }
    return p;
}

double elapsed_ms(const Clock::time_point& t0, const Clock::time_point& t1) {
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}
} // namespace

namespace bench_external {

double run_ffht_single_frame_ms(const std::vector<float>& input, std::size_t iterations) {
    if (input.empty()) {
        throw std::invalid_argument("FFHT input is empty");
    }
    if (!is_power_of_two(input.size())) {
        throw std::invalid_argument("FFHT input size must be a power of two");
    }
    if (iterations == 0) {
        throw std::invalid_argument("iterations must be > 0");
    }

    const int log_n = ilog2_exact(input.size());

    for (int i = 0; i < 10; ++i) {
        std::vector<float> tmp = input;
        const int rc = fht_float(tmp.data(), log_n);
        if (rc != 0) {
            throw std::runtime_error("fht_float failed during warmup");
        }
    }

    const auto t0 = Clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        std::vector<float> tmp = input;
        const int rc = fht_float(tmp.data(), log_n);
        if (rc != 0) {
            throw std::runtime_error("fht_float failed");
        }
    }
    const auto t1 = Clock::now();

    return elapsed_ms(t0, t1) / static_cast<double>(iterations);
}

double run_fftw_r2c_single_frame_ms(const std::vector<float>& input, std::size_t iterations) {
    if (input.empty()) {
        throw std::invalid_argument("FFTW input is empty");
    }
    if (iterations == 0) {
        throw std::invalid_argument("iterations must be > 0");
    }

    const int n = static_cast<int>(input.size());

    std::vector<float> in_copy(n, 0.0f);
    fftwf_complex* out =
        reinterpret_cast<fftwf_complex*>(fftwf_malloc(sizeof(fftwf_complex) * (n / 2 + 1)));

    if (!out) {
        throw std::runtime_error("fftwf_malloc failed");
    }

    fftwf_plan plan = fftwf_plan_dft_r2c_1d(
        n,
        in_copy.data(),
        out,
        FFTW_MEASURE
    );

    if (!plan) {
        fftwf_free(out);
        throw std::runtime_error("fftwf_plan_dft_r2c_1d failed");
    }

    for (int i = 0; i < 10; ++i) {
        std::memcpy(in_copy.data(), input.data(), sizeof(float) * input.size());
        fftwf_execute(plan);
    }

    const auto t0 = Clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        std::memcpy(in_copy.data(), input.data(), sizeof(float) * input.size());
        fftwf_execute(plan);
    }
    const auto t1 = Clock::now();

    fftwf_destroy_plan(plan);
    fftwf_free(out);

    return elapsed_ms(t0, t1) / static_cast<double>(iterations);
}

} // namespace bench_external