#include "reconstruction.hpp"

#include "wht.hpp"
#include "window.hpp"

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

    for (std::size_t m = 0; m < frames.frames; ++m) {
        std::vector<float> y(frames.bins, 0.0f);
        for (std::size_t k = 0; k < frames.bins; ++k) {
            y[k] = frames.coeffs[m * frames.bins + k];
        }

        auto x = wht::inverse(y, ordering, orthonormal);

        const std::size_t start = m * frames.hop;
        for (std::size_t i = 0; i < frames.frame_size; ++i) {
            const float v = x[i] * win[i];
            out.signal[start + i] += v;
            out.window_mass[start + i] += win[i] * win[i];
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