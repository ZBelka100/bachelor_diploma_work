#include "metrics.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <vector>

TEST(MetricsTest, EnergyAndRms) {
    const std::vector<float> x = {3.0f, 4.0f};

    EXPECT_NEAR(metrics::energy(x), 25.0, 1e-9);
    EXPECT_NEAR(metrics::rms(x), std::sqrt(12.5), 1e-9);
}

TEST(MetricsTest, CompareIdenticalSignals) {
    const std::vector<float> x = {0.0f, 0.5f, -0.25f, 1.0f};

    auto m = metrics::compare(x, x);

    EXPECT_NEAR(m.mse, 0.0, 1e-12);
    EXPECT_NEAR(m.rmse, 0.0, 1e-12);
    EXPECT_NEAR(m.max_abs, 0.0, 1e-12);
    EXPECT_GT(m.snr_db, 200.0);
}

TEST(MetricsTest, CompareRejectsSizeMismatch) {
    const std::vector<float> a = {1.0f, 2.0f};
    const std::vector<float> b = {1.0f};

    EXPECT_THROW(metrics::compare(a, b), std::invalid_argument);
}

TEST(MetricsTest, EnergyCompactionTopK) {
    const std::vector<float> coeffs = {2.0f, 1.0f, 0.0f, 1.0f};

    const double value = metrics::energy_compaction_top_k(coeffs, 1);

    EXPECT_NEAR(value, 4.0 / 6.0, 1e-9);
}

TEST(MetricsTest, PeakToTotalRatio) {
    const std::vector<float> coeffs = {2.0f, 1.0f, 0.0f, 1.0f};

    const double value = metrics::peak_to_total_ratio(coeffs);

    EXPECT_NEAR(value, 4.0 / 6.0, 1e-9);
}