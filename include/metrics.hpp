#pragma once

#include "types.hpp"

#include <cstddef>
#include <vector>

namespace metrics {

double energy(const std::vector<float>& x);
double rms(const std::vector<float>& x);

ErrorMetrics compare(
    const std::vector<float>& reference,
    const std::vector<float>& test
);

double energy_compaction_top_k(
    const std::vector<float>& coeffs,
    std::size_t k
);

double peak_to_total_ratio(
    const std::vector<float>& coeffs
);

} // namespace metrics