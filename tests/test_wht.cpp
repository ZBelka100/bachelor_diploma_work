#include "metrics.hpp"
#include "wht.hpp"

#include <cmath>
#include <iostream>
#include <vector>

int main() {
    std::vector<float> x = {1.0f, -2.0f, 3.0f, 0.5f, -1.5f, 2.0f, 4.0f, -0.25f};

    const auto y = wht::forward(x, Ordering::Sequency, true);
    const auto xr = wht::inverse(y, Ordering::Sequency, true);

    const auto m = metrics::compare(x, xr);

    std::cout << "WHT inverse test\n";
    std::cout << "MSE:    " << m.mse << "\n";
    std::cout << "MaxAbs: " << m.max_abs << "\n";
    std::cout << "SNR:    " << m.snr_db << " dB\n";

    if (m.max_abs > 1e-4) {
        std::cerr << "FAIL: reconstruction error too large\n";
        return 1;
    }

    const double ex = metrics::energy(x);
    const double ey = metrics::energy(y);
    const double rel = std::abs(ex - ey) / std::max(1.0, ex);

    std::cout << "Energy x: " << ex << "\n";
    std::cout << "Energy y: " << ey << "\n";
    std::cout << "Rel diff: " << rel << "\n";

    if (rel > 1e-4) {
        std::cerr << "FAIL: energy mismatch\n";
        return 1;
    }

    std::cout << "PASS\n";
    return 0;
}