#include "metrics.hpp"
#include "pseudospectrum.hpp"
#include "reconstruction.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

TEST(ReconstructionTest, ReconstructsWhtFrames) {
    const int sample_rate = 8000;
    std::vector<float> signal(2048, 0.0f);

    for (std::size_t i = 0; i < signal.size(); ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        signal[i] = 0.4f * std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * t);
    }

    const auto frames = pseudospectrum::compute_wht_frames(
        signal,
        sample_rate,
        256,
        128,
        WindowType::SqrtHann,
        Ordering::Sequency,
        true
    );

    const auto rec = reconstruction::from_wht_frames(
        frames,
        Ordering::Sequency,
        WindowType::SqrtHann,
        true
    );

    const std::size_t n = std::min(signal.size(), rec.signal.size());

    const std::vector<float> ref(signal.begin(), signal.begin() + n);
    const std::vector<float> out(rec.signal.begin(), rec.signal.begin() + n);

    const auto err = metrics::compare(ref, out);

    EXPECT_LT(err.rmse, 1e-3);
    EXPECT_GT(err.snr_db, 40.0);
}