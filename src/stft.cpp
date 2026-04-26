#include "stft.hpp"

#include "framing.hpp"
#include "window.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

namespace stft {

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEps = 1e-12f;

float safe_log10(float x) {
    return std::log10(std::max(x, kEps));
}

bool is_power_of_two(std::size_t n) {
    return n != 0 && ((n & (n - 1)) == 0);
}

std::size_t next_power_of_two(std::size_t n) {
    if (n == 0) return 1;
    std::size_t p = 1;
    while (p < n) p <<= 1U;
    return p;
}

void fft_inplace(std::vector<std::complex<float>>& a, bool inverse) {
    const std::size_t n = a.size();
    if (!is_power_of_two(n)) {
        throw std::invalid_argument("stft::fft_inplace: n must be power of two");
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

ComplexFrameTransform compute_frames(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type
) {
    if (sample_rate <= 0) {
        throw std::invalid_argument("stft::compute_frames: sample_rate must be positive");
    }
    if (frame_size == 0) {
        throw std::invalid_argument("stft::compute_frames: frame_size must be positive");
    }
    if (hop == 0) {
        throw std::invalid_argument("stft::compute_frames: hop must be positive");
    }

    const std::size_t frames = framing::count_frames(signal.size(), frame_size, hop);
    const std::size_t nfft = next_power_of_two(frame_size);
    const std::size_t bins = nfft / 2 + 1;
    const auto win = window::make(window_type, frame_size);

    ComplexFrameTransform out;
    out.frames = frames;
    out.bins = bins;
    out.frame_size = frame_size;
    out.transform_size = nfft;
    out.hop = hop;
    out.sample_rate = sample_rate;
    out.coeffs.assign(frames * bins, std::complex<float>(0.0f, 0.0f));

    std::vector<std::complex<float>> buf(nfft, std::complex<float>(0.0f, 0.0f));

    for (std::size_t m = 0; m < frames; ++m) {
        const std::size_t start = m * hop;
        std::fill(buf.begin(), buf.end(), std::complex<float>(0.0f, 0.0f));

        for (std::size_t i = 0; i < frame_size; ++i) {
            const std::size_t pos = start + i;
            if (pos < signal.size()) {
                buf[i] = std::complex<float>(signal[pos] * win[i], 0.0f);
            }
        }

        fft_inplace(buf, false);

        for (std::size_t k = 0; k < bins; ++k) {
            out.coeffs[m * bins + k] = buf[k];
        }
    }

    return out;
}

Spectrogram compute_from_frames(const ComplexFrameTransform& frames_in) {
    if (frames_in.frames == 0) {
        throw std::invalid_argument("stft::compute_from_frames: no frames");
    }
    if (frames_in.bins == 0) {
        throw std::invalid_argument("stft::compute_from_frames: no bins");
    }
    if (frames_in.coeffs.size() != frames_in.frames * frames_in.bins) {
        throw std::invalid_argument("stft::compute_from_frames: coeff size mismatch");
    }
    if (frames_in.sample_rate <= 0) {
        throw std::invalid_argument("stft::compute_from_frames: invalid sample_rate");
    }
    if (frames_in.transform_size == 0) {
        throw std::invalid_argument("stft::compute_from_frames: invalid transform_size");
    }

    Spectrogram out;
    out.frames = frames_in.frames;
    out.bins = frames_in.bins;
    out.frame_size = frames_in.frame_size;
    out.transform_size = frames_in.transform_size;
    out.hop = frames_in.hop;
    out.sample_rate = frames_in.sample_rate;

    out.power.assign(out.frames * out.bins, 0.0f);
    out.db.assign(out.frames * out.bins, 0.0f);
    out.time_s.resize(out.frames, 0.0f);
    out.pseudo_freq_hz.resize(out.bins, 0.0f);

    float global_max_power = 0.0f;

    for (std::size_t m = 0; m < out.frames; ++m) {
        out.time_s[m] =
            static_cast<float>(m * out.hop) / static_cast<float>(out.sample_rate);

        const std::size_t row = m * out.bins;
        for (std::size_t k = 0; k < out.bins; ++k) {
            const float p = std::norm(frames_in.coeffs[row + k]);
            out.power[row + k] = p;
            global_max_power = std::max(global_max_power, p);
        }
    }

    const float ref = std::max(global_max_power, kEps);
    for (std::size_t i = 0; i < out.power.size(); ++i) {
        out.db[i] = 10.0f * safe_log10(out.power[i] / ref);
    }

    for (std::size_t k = 0; k < out.bins; ++k) {
        out.pseudo_freq_hz[k] =
            static_cast<float>(k) * static_cast<float>(out.sample_rate) /
            static_cast<float>(out.transform_size);
    }

    return out;
}

Spectrogram compute(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type
) {
    const auto frames = compute_frames(signal, sample_rate, frame_size, hop, window_type);
    return compute_from_frames(frames);
}

} // namespace stft