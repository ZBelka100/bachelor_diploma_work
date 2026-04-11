#include "reconstruction.hpp"

#include "wht.hpp"
#include "window.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

namespace reconstruction {

ReconstructionResult from_wht_frames(
    const FrameTransform& frames,
    Ordering ordering,
    WindowType synthesis_window,
    bool orthonormal
) {
    if (frames.frames == 0 || frames.bins == 0 || frames.frame_size == 0 || frames.hop == 0) {
        throw std::invalid_argument("reconstruction: invalid frame transform metadata");
    }
    if (frames.coeffs.size() != frames.frames * frames.bins) {
        throw std::invalid_argument("reconstruction: coeff size mismatch");
    }
    if (frames.frame_size > frames.bins) {
        throw std::invalid_argument("reconstruction: frame_size must be <= bins");
    }

    const std::size_t out_size = (frames.frames - 1) * frames.hop + frames.frame_size;

    ReconstructionResult out;
    out.signal.assign(out_size, 0.0f);
    out.window_mass.assign(out_size, 0.0f);

    const auto win = window::make(synthesis_window, frames.frame_size);
    const float* win_ptr = win.data();

    wht::Plan plan(frames.bins, ordering, orthonormal);
    std::vector<float> buf(frames.bins, 0.0f);

    const float* coeffs = frames.coeffs.data();

    for (std::size_t m = 0; m < frames.frames; ++m) {
        const std::size_t row = m * frames.bins;
        std::memcpy(buf.data(), coeffs + row, frames.bins * sizeof(float));

        wht::inverse_inplace(buf.data(), plan);

        const std::size_t start = m * frames.hop;
        for (std::size_t i = 0; i < frames.frame_size; ++i) {
            const float w = win_ptr[i];
            const float v = buf[i] * w;
            out.signal[start + i] += v;
            out.window_mass[start + i] += w * w;
        }
    }

    for (std::size_t i = 0; i < out.signal.size(); ++i) {
        if (out.window_mass[i] > 1e-12f) {
            out.signal[i] /= out.window_mass[i];
        }
    }

    return out;
}

} // namespace reconstruction