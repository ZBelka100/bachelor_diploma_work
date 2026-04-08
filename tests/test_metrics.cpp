#include "metrics.hpp"

#include <iostream>
#include <vector>

int main() {
    const std::vector<float> x = {1, 2, 3, 4};
    const std::vector<float> y = {1, 2, 3, 4};
    const std::vector<float> z = {1, 2, 2, 4};

    const auto m1 = metrics::compare(x, y);
    const auto m2 = metrics::compare(x, z);

    std::cout << "Compare identical:\n";
    std::cout << "  MSE: " << m1.mse << "\n";
    std::cout << "  SNR: " << m1.snr_db << "\n";

    std::cout << "Compare modified:\n";
    std::cout << "  MSE: " << m2.mse << "\n";
    std::cout << "  SNR: " << m2.snr_db << "\n";

    const std::vector<float> coeffs = {5.0f, 0.1f, 0.2f, 0.1f};
    const double c1 = metrics::energy_compaction_top_k(coeffs, 1);
    const double ptr = metrics::peak_to_total_ratio(coeffs);

    std::cout << "Top-1 compaction: " << c1 << "\n";
    std::cout << "PTR: " << ptr << "\n";

    if (m1.mse != 0.0) return 1;
    if (m2.mse <= 0.0) return 1;
    if (c1 <= 0.9) return 1;

    std::cout << "PASS\n";
    return 0;
}