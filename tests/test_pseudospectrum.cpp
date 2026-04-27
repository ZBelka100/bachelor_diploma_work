#include "pseudospectrum.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <vector>

TEST(PseudospectrumTest, ComputesWhtFrames) {
    const int sample_rate = 8000;
    std::vector<float> signal(64, 0.0f);

    for (std::size_t i = 0; i < signal.size(); ++i) {
        signal[i] = static_cast<float>(
            std::sin(2.0 * M_PI * 440.0 * static_cast<double>(i) / sample_rate)
        );
    }

    const auto frames = pseudospectrum::compute_wht_frames(
        signal,
        sample_rate,
        16,
        8,
        WindowType::Hann,
        Ordering::Sequency,
        true
    );

    EXPECT_EQ(frames.frames, 8);
    EXPECT_EQ(frames.bins, 16);
    EXPECT_EQ(frames.frame_size, 16);
    EXPECT_EQ(frames.transform_size, 16);
    EXPECT_EQ(frames.hop, 8);
    EXPECT_EQ(frames.sample_rate, sample_rate);
    EXPECT_EQ(frames.coeffs.size(), frames.frames * frames.bins);
}

TEST(PseudospectrumTest, BuildsSpectrogramFromFrames) {
    const int sample_rate = 8000;
    std::vector<float> signal(64, 0.0f);

    for (std::size_t i = 0; i < signal.size(); ++i) {
        signal[i] = static_cast<float>(
            std::sin(2.0 * M_PI * 440.0 * static_cast<double>(i) / sample_rate)
        );
    }

    const auto frames = pseudospectrum::compute_wht_frames(
        signal,
        sample_rate,
        16,
        8,
        WindowType::Hann,
        Ordering::Sequency,
        true
    );

    const auto spec = pseudospectrum::compute_from_frames(frames);

    EXPECT_EQ(spec.frames, frames.frames);
    EXPECT_EQ(spec.bins, frames.bins);
    EXPECT_EQ(spec.power.size(), spec.frames * spec.bins);
    EXPECT_EQ(spec.db.size(), spec.frames * spec.bins);
    EXPECT_EQ(spec.time_s.size(), spec.frames);
    EXPECT_EQ(spec.pseudo_freq_hz.size(), spec.bins);
    EXPECT_LE(spec.db[0], 0.0f);
}

TEST(PseudospectrumTest, RejectsInvalidParameters) {
    const std::vector<float> signal(64, 0.0f);

    EXPECT_THROW(
        pseudospectrum::compute_wht_frames(
            signal,
            0,
            16,
            8,
            WindowType::Hann,
            Ordering::Sequency,
            true
        ),
        std::invalid_argument
    );
}