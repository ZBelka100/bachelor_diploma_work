#include "wht.hpp"
#include "metrics.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

int main() {
    std::vector<float> x = {1.0f, -2.0f, 3.0f, 0.5f, -1.5f, 2.0f, 4.0f, -0.25f};
    auto y = wht::forward(x, Ordering::Sequency, true);
    auto xr = wht::inverse(y, Ordering::Sequency, true);
    auto m = metrics::compare(x, xr);
    std::cout << "WHT inverse test (Sequency)\n";
    std::cout << "MSE: " << m.mse << ", MaxAbs: " << m.max_abs << ", SNR: " << m.snr_db << " dB\n";
    if (m.max_abs > 1e-4 || m.mse > 1e-8) {
        std::cerr << "FAIL: reconstruction error too large\n";
        return 1;
    }

    x = {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    auto y_had = wht::forward(x, Ordering::Hadamard, true);
    auto xr_had = wht::inverse(y_had, Ordering::Hadamard, true);
    m = metrics::compare(x, xr_had);
    std::cout << "WHT inverse test (Hadamard)\n";
    std::cout << "MSE: " << m.mse << ", MaxAbs: " << m.max_abs << ", SNR: " << m.snr_db << " dB\n";
    if (m.max_abs > 1e-4 || m.mse > 1e-8) {
        std::cerr << "FAIL: reconstruction error too large\n";
        return 1;
    }

    std::vector<float> signal = {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    auto coeffs = wht::forward(signal, Ordering::Sequency, true);
    double energy_in = metrics::energy(signal);
    double energy_out = metrics::energy(coeffs);
    double rel_diff = std::abs(energy_in - energy_out) / std::max(1.0, energy_in);
    std::cout << "Energy test\n";
    std::cout << "Energy in: " << energy_in << ", Energy out: " << energy_out << ", Rel diff: " << rel_diff << "\n";
    if (rel_diff > 1e-4) {
        std::cerr << "FAIL: energy mismatch\n";
        return 1;
    }

    std::vector<float> a = {1.0f, 0.0f, 0.0f, 0.0f};
    auto fa = wht::forward(a, Ordering::Hadamard, true);
    std::vector<float> b = {0.0f, 1.0f, 0.0f, 0.0f};
    auto fb = wht::forward(b, Ordering::Hadamard, true);
    double dot_orig = 0.0;
    for (size_t i = 0; i < a.size(); ++i) dot_orig += a[i] * b[i];
    double dot_trans = 0.0;
    for (size_t i = 0; i < fa.size(); ++i) dot_trans += fa[i] * fb[i];
    std::cout << "Orthonormality test\n";
    std::cout << "Dot original: " << dot_orig << ", Dot transformed: " << dot_trans << "\n";
    if (std::abs(dot_orig - dot_trans) > 1e-4) {
        std::cerr << "FAIL: orthonormality violation\n";
        return 1;
    }

    if (!wht::is_power_of_two(8) || wht::is_power_of_two(7)) {
        std::cerr << "FAIL: is_power_of_two error\n";
        return 1;
    }
    if (wht::next_power_of_two(5) != 8 || wht::next_power_of_two(8) != 8) {
        std::cerr << "FAIL: next_power_of_two error\n";
        return 1;
    }

    try {
        wht::Plan bad_plan(7, Ordering::Sequency, true);
        std::cerr << "FAIL: expected exception for n=7\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "PASS\n";
    return 0;
}
