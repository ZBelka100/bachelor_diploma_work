#pragma once

#include "types.hpp"

#include <cstddef>
#include <vector>

namespace pseudospectrum {

FrameTransform compute_wht_frames(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type = WindowType::SqrtHann,
    Ordering ordering = Ordering::Sequency,
    bool orthonormal = true
);

Spectrogram compute_from_frames(const FrameTransform& frames);

Spectrogram compute(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type = WindowType::SqrtHann,
    Ordering ordering = Ordering::Sequency,
    bool orthonormal = true
);

} // namespace pseudospectrum