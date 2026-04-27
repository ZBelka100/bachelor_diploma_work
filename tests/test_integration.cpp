#include "metrics.hpp"
#include "pseudospectrum.hpp"
#include "reconstruction.hpp"
#include "stft.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

TEST(IntegrationTest, FullWhtAndStftPipelineRuns) {
    const int sample_rate = 16000;
    const std::size_t n = 4096;

    std::vector<float> signal(n, 0.0f);

    for (std::size_t i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        signal[i] = 0.5f * std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * t);
    }

    const std::size_t frame_size = 512;
    const std::size_t hop = 256;
    const auto window_type = WindowType::SqrtHann;

    const auto wht_frames = pseudospectrum::compute_wht_frames(
        signal,
        sample_rate,
        frame_size,
        hop,
        window_type,
        Ordering::Sequency,
        true
    );

    const auto wht_spec = pseudospectrum::compute_from_frames(wht_frames);

    EXPECT_EQ(wht_spec.frames, wht_frames.frames);
    EXPECT_EQ(wht_spec.bins, wht_frames.bins);
    EXPECT_FALSE(wht_spec.db.empty());

    const auto wht_rec = reconstruction::from_wht_frames(
        wht_frames,
        Ordering::Sequency,
        window_type,
        true
    );

    const std::size_t common = std::min(signal.size(), wht_rec.signal.size());

    const std::vector<float> ref(signal.begin(), signal.begin() + common);
    const std::vector<float> rec(wht_rec.signal.begin(), wht_rec.signal.begin() + common);

    const auto err = metrics::compare(ref, rec);

    EXPECT_LT(err.rmse, 1e-3);
    EXPECT_GT(err.snr_db, 40.0);

    const auto stft_spec = stft::compute(
        signal,
        sample_rate,
        frame_size,
        hop,
        window_type
    );

    EXPECT_GT(stft_spec.frames, 0U);
    EXPECT_GT(stft_spec.bins, 0U);
    EXPECT_FALSE(stft_spec.db.empty());
}