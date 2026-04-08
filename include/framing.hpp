#pragma once

#include <cstddef>
#include <vector>

namespace framing {

// Extract one frame of length frame_size from x[start...], zero-pad tail.
std::vector<float> make_frame(
    const std::vector<float>& x,
    std::size_t start,
    std::size_t frame_size
);

// Number of frames with tail included.
std::size_t count_frames(
    std::size_t signal_size,
    std::size_t frame_size,
    std::size_t hop
);

} // namespace framing