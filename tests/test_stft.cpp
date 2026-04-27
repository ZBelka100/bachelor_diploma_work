#include "stft.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <vector>

TEST(StftTest, ComputesSpectrogram) {
    const int sample_rate = 8000;
    std::vector<float> signal(512, 0.0f);

    for (std::size_t i = 0; i < signal.size(); ++i) {
        signal[i] = static_cast<float>(
            std::sin(2.0 * M_PI * 440.0 * static_cast<double>(i) / sample_rate)
        );
    }

    const auto spec = stft::compute(signal, sample_rate, 128, 64, WindowType::Hann);

    EXPECT_EQ(spec.frames, 8);
    EXPECT_EQ(spec.bins, spec.transform_size / 2 + 1);
    EXPECT_EQ(spec.frame_size, 128);
    EXPECT_EQ(spec.hop, 64);
    EXPECT_EQ(spec.sample_rate, sample_rate);
    EXPECT_EQ(spec.power.size(), spec.frames * spec.bins);
    EXPECT_EQ(spec.db.size(), spec.frames * spec.bins);
    EXPECT_EQ(spec.time_s.size(), spec.frames);
    EXPECT_EQ(spec.pseudo_freq_hz.size(), spec.bins);
    EXPECT_FLOAT_EQ(spec.pseudo_freq_hz[0], 0.0f);
    EXPECT_LE(spec.db[0], 0.0f);
}

TEST(StftTest, RejectsInvalidParameters) {
    const std::vector<float> signal(512, 0.0f);

    EXPECT_THROW(
        stft::compute(signal, 0, 128, 64, WindowType::Hann),
        std::invalid_argument
    );
}