#include "reconstruction.hpp"

#include "wht.hpp"
#include "window.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

namespace reconstruction {

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEps = 1e-12f;

bool is_power_of_two(std::size_t n) {
    return n != 0 && ((n & (n - 1)) == 0);
}

void fft_inplace(std::vector<std::complex<float>>& a, bool inverse) {
    const std::size_t n = a.size();
    if (!is_power_of_two(n)) {
        throw std::invalid_argument("reconstruction::fft_inplace: n must be power of two");
    }

    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1U;
        for (; j & bit; bit >>= 1U) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }

    for (std::size_t len = 2; len <= n; len <<= 1U) {
        const float ang =
            2.0f * kPi / static_cast<float>(len) * (inverse ? 1.0f : -1.0f);
        const std::complex<float> wlen(std::cos(ang), std::sin(ang));

        for (std::size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (std::size_t j = 0; j < len / 2; ++j) {
                const auto u = a[i + j];
                const auto v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse) {
        const float inv_n = 1.0f / static_cast<float>(n);
        for (auto& x : a) {
            x *= inv_n;
        }
    }
}

} // namespace

ReconstructionResult from_wht_frames(
    const FrameTransform& frames,
    Ordering ordering,
    WindowType synthesis_window,
    bool orthonormal
) {
    if (frames.frames == 0 || frames.bins == 0 || frames.frame_size == 0 || frames.hop == 0) {
        throw std::invalid_argument("reconstruction::from_wht_frames: invalid metadata");
    }
    if (frames.coeffs.size() != frames.frames * frames.bins) {
        throw std::invalid_argument("reconstruction::from_wht_frames: coeff size mismatch");
    }
    if (frames.frame_size > frames.bins) {
        throw std::invalid_argument("reconstruction::from_wht_frames: frame_size must be <= bins");
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

        const auto x = wht::inverse(y, ordering, orthonormal);

        const std::size_t start = m * frames.hop;
        for (std::size_t i = 0; i < frames.frame_size; ++i) {
            const float v = x[i] * win[i];
            out.signal[start + i] += v;
            out.window_mass[start + i] += win[i] * win[i];
        }
    }

    for (std::size_t i = 0; i < out.signal.size(); ++i) {
        if (out.window_mass[i] > kEps) {
            out.signal[i] /= out.window_mass[i];
        }
    }

    return out;
}

ReconstructionResult from_stft_frames(
    const stft::ComplexFrameTransform& frames,
    WindowType synthesis_window
) {
    if (frames.frames == 0 || frames.bins == 0 || frames.frame_size == 0 || frames.hop == 0) {
        throw std::invalid_argument("reconstruction::from_stft_frames: invalid metadata");
    }
    if (frames.transform_size == 0) {
        throw std::invalid_argument("reconstruction::from_stft_frames: invalid transform_size");
    }
    if (frames.coeffs.size() != frames.frames * frames.bins) {
        throw std::invalid_argument("reconstruction::from_stft_frames: coeff size mismatch");
    }
    if (frames.bins != frames.transform_size / 2 + 1) {
        throw std::invalid_argument("reconstruction::from_stft_frames: bins/transform_size mismatch");
    }
    if (frames.frame_size > frames.transform_size) {
        throw std::invalid_argument("reconstruction::from_stft_frames: frame_size > transform_size");
    }

    const std::size_t nfft = frames.transform_size;
    const std::size_t bins = frames.bins;
    const std::size_t out_size = (frames.frames - 1) * frames.hop + frames.frame_size;

    ReconstructionResult out;
    out.signal.assign(out_size, 0.0f);
    out.window_mass.assign(out_size, 0.0f);

    const auto win = window::make(synthesis_window, frames.frame_size);
    std::vector<std::complex<float>> full(nfft, std::complex<float>(0.0f, 0.0f));

    for (std::size_t m = 0; m < frames.frames; ++m) {
        std::fill(full.begin(), full.end(), std::complex<float>(0.0f, 0.0f));

        const std::size_t row = m * bins;

        for (std::size_t k = 0; k < bins; ++k) {
            full[k] = frames.coeffs[row + k];
        }

        // Восстановление эрмитовой симметрии для вещественного сигнала
        for (std::size_t k = 1; k + 1 < bins; ++k) {
            full[nfft - k] = std::conj(full[k]);
        }

        fft_inplace(full, true);

        const std::size_t start = m * frames.hop;
        for (std::size_t i = 0; i < frames.frame_size; ++i) {
            const float v = full[i].real() * win[i];
            out.signal[start + i] += v;
            out.window_mass[start + i] += win[i] * win[i];
        }
    }

    for (std::size_t i = 0; i < out.signal.size(); ++i) {
        if (out.window_mass[i] > kEps) {
            out.signal[i] /= out.window_mass[i];
        }
    }

    return out;
}

} // namespace reconstruction