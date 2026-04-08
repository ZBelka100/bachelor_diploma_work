#include "metrics.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace metrics {

double energy(const std::vector<float>& x) {
    double e = 0.0;
    for (float v : x) e += static_cast<double>(v) * static_cast<double>(v);
    return e;
}

double rms(const std::vector<float>& x) {
    if (x.empty()) return 0.0;
    return std::sqrt(energy(x) / static_cast<double>(x.size()));
}

ErrorMetrics compare(
    const std::vector<float>& reference,
    const std::vector<float>& test
) {
    if (reference.size() != test.size()) {
        throw std::invalid_argument("metrics::compare: size mismatch");
    }

    ErrorMetrics m{};
    if (reference.empty()) return m;

    double se = 0.0;
    double ref_e = 0.0;
    double max_abs = 0.0;

    for (std::size_t i = 0; i < reference.size(); ++i) {
        const double d = static_cast<double>(reference[i]) - static_cast<double>(test[i]);
        se += d * d;
        ref_e += static_cast<double>(reference[i]) * static_cast<double>(reference[i]);
        max_abs = std::max(max_abs, std::abs(d));
    }

    m.mse = se / static_cast<double>(reference.size());
    m.rmse = std::sqrt(m.mse);
    m.max_abs = max_abs;

    if (se <= 1e-30) {
        m.snr_db = 300.0;
    } else {
        m.snr_db = 10.0 * std::log10(std::max(ref_e, 1e-30) / se);
    }

    return m;
}

double energy_compaction_top_k(
    const std::vector<float>& coeffs,
    std::size_t k
) {
    if (coeffs.empty()) return 0.0;
    k = std::min(k, coeffs.size());

    std::vector<double> p;
    p.reserve(coeffs.size());

    double total = 0.0;
    for (float c : coeffs) {
        const double e = static_cast<double>(c) * static_cast<double>(c);
        p.push_back(e);
        total += e;
    }

    std::nth_element(p.begin(), p.begin() + static_cast<std::ptrdiff_t>(k), p.end(), std::greater<>());
    std::sort(p.begin(), p.begin() + static_cast<std::ptrdiff_t>(k), std::greater<>());

    double top = 0.0;
    for (std::size_t i = 0; i < k; ++i) top += p[i];

    if (total <= 1e-30) return 0.0;
    return top / total;
}

double peak_to_total_ratio(const std::vector<float>& coeffs) {
    if (coeffs.empty()) return 0.0;

    double total = 0.0;
    double peak = 0.0;

    for (float c : coeffs) {
        const double e = static_cast<double>(c) * static_cast<double>(c);
        total += e;
        peak = std::max(peak, e);
    }

    if (total <= 1e-30) return 0.0;
    return peak / total;
}

} // namespace metrics