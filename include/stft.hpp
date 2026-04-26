#pragma once

#include "types.hpp"

#include <complex>
#include <vector>

namespace stft {

struct ComplexFrameTransform {
    std::size_t frames = 0;
    std::size_t bins = 0;           // unique bins = nfft / 2 + 1
    std::size_t frame_size = 0;
    std::size_t transform_size = 0; // nfft
    std::size_t hop = 0;
    int sample_rate = 0;

    // row-major: frames x bins
    std::vector<std::complex<float>> coeffs;
};

ComplexFrameTransform compute_frames(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type = WindowType::Hann
);

Spectrogram compute_from_frames(const ComplexFrameTransform& frames);

Spectrogram compute(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type = WindowType::Hann
);

} // namespace stft