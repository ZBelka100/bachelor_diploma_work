#include "framing.hpp"

#include <stdexcept>

namespace framing {

std::vector<float> make_frame(
    const std::vector<float>& x,
    std::size_t start,
    std::size_t frame_size
) {
    std::vector<float> frame(frame_size, 0.0f);
    for (std::size_t i = 0; i < frame_size; ++i) {
        const std::size_t pos = start + i;
        if (pos < x.size()) {
            frame[i] = x[pos];
        }
    }
    return frame;
}

std::size_t count_frames(
    std::size_t signal_size,
    std::size_t frame_size,
    std::size_t hop
) {
    if (frame_size == 0 || hop == 0) {
        throw std::invalid_argument("count_frames: frame_size and hop must be > 0");
    }
    if (signal_size == 0) return 0;
    return (signal_size + hop - 1) / hop;
}

} // namespace framing