#include "metrics.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

int main() {
    std::vector<float> x = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> y = {1.0f, 2.0f, 3.0f, 4.0f};
    auto m = metrics::compare(x, y);
    std::cout << "Compare identical\n";
    std::cout << "MSE: " << m.mse << ", RMSE: " << m.rmse << ", MaxAbs: " << m.max_abs << ", SNR: " << m.snr_db << " dB\n";
    if (m.mse != 0.0 || m.rmse != 0.0 || m.max_abs != 0.0) {
        std::cerr << "FAIL: identical signals should have zero error\n";
        return 1;
    }

    std::vector<float> z = {1.0f, 2.0f, 2.0f, 4.0f};
    m = metrics::compare(x, z);
    std::cout << "Compare different\n";
    std::cout << "MSE: " << m.mse << ", RMSE: " << m.rmse << ", MaxAbs: " << m.max_abs << ", SNR: " << m.snr_db << " dB\n";
    if (m.mse <= 0.0 || m.max_abs <= 0.0) {
        std::cerr << "FAIL: different signals should have non-zero error\n";
        return 1;
    }

    std::vector<float> short_vec = {1.0f, 2.0f};
    try {
        metrics::compare(x, short_vec);
        std::cerr << "FAIL: expected exception for size mismatch\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }

    double eng = metrics::energy(x);
    double expected_eng = 1.0*1.0 + 2.0*2.0 + 3.0*3.0 + 4.0*4.0;
    std::cout << "Energy: " << eng << " (expected " << expected_eng << ")\n";
    if (std::abs(eng - expected_eng) > 1e-12) {
        std::cerr << "FAIL: energy calculation error\n";
        return 1;
    }

    double rms_val = metrics::rms(x);
    double expected_rms = std::sqrt(expected_eng / 4.0);
    std::cout << "RMS: " << rms_val << " (expected " << expected_rms << ")\n";
    if (std::abs(rms_val - expected_rms) > 1e-12) {
        std::cerr << "FAIL: RMS calculation error\n";
        return 1;
    }

    std::vector<float> coeffs = {5.0f, 0.1f, 0.2f, 0.1f};
    double c1 = metrics::energy_compaction_top_k(coeffs, 1);
    std::cout << "Top-1 compaction: " << c1 << "\n";
    if (c1 < 0.9) {
        std::cerr << "FAIL: compaction ratio too low\n";
        return 1;
    }
    if (std::abs(c1 - 0.9976) > 0.01) {
        std::cerr << "FAIL: compaction ratio incorrect\n";
        return 1;
    }

    double ptr = metrics::peak_to_total_ratio(coeffs);
    double expected_ptr = 25.0 / 25.06;
    std::cout << "PTR: " << ptr << " (expected " << expected_ptr << ")\n";
    if (std::abs(ptr - expected_ptr) > 0.01) {
        std::cerr << "FAIL: PTR incorrect\n";
        return 1;
    }

    x = {1.0f};
    y = {1.0f};
    m = metrics::compare(x, y);
    std::cout << "SNR for perfect match: " << m.snr_db << " dB\n";
    if (m.snr_db < 200.0) {
        std::cerr << "FAIL: SNR should be very high for perfect match\n";
        return 1;
    }

    std::cout << "PASS\n";
    return 0;
}
