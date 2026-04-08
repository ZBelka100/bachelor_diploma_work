#pragma once

#include "types.hpp"

#include <cstddef>
#include <vector>

namespace stft {

Spectrogram compute(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type = WindowType::Hann
);

} // namespace stft