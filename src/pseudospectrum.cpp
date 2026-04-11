#include "pseudospectrum.hpp"

#include "framing.hpp"
#include "wht.hpp"
#include "window.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace pseudospectrum {

static inline float safe_log10(float x) {
    constexpr float eps = 1e-12f;
    return std::log10(std::max(x, eps));
}

FrameTransform compute_wht_frames(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type,
    Ordering ordering,
    bool orthonormal
) {
    if (sample_rate <= 0) {
        throw std::invalid_argument("compute_wht_frames: sample_rate must be > 0");
    }
    if (frame_size == 0 || hop == 0) {
        throw std::invalid_argument("compute_wht_frames: frame_size and hop must be > 0");
    }

    const std::size_t n = wht::next_power_of_two(frame_size);
    const std::size_t frames = framing::count_frames(signal.size(), frame_size, hop);
    const auto win = window::make(window_type, frame_size);
    wht::Plan plan(n, ordering, orthonormal);

    FrameTransform out;
    out.frames = frames;
    out.bins = n;
    out.frame_size = frame_size;
    out.transform_size = n;
    out.hop = hop;
    out.sample_rate = sample_rate;
    out.coeffs.assign(frames * n, 0.0f);

    std::vector<float> buf(n, 0.0f);

    const float* sig = signal.data();
    const float* win_ptr = win.data();
    float* coeffs_ptr = out.coeffs.data();

    for (std::size_t m = 0; m < frames; ++m) {
        const std::size_t start = m * hop;
        float* dst = coeffs_ptr + m * n;

        std::fill(buf.begin(), buf.end(), 0.0f);

        const std::size_t available =
            (start < signal.size()) ? std::min(frame_size, signal.size() - start) : 0;

        for (std::size_t i = 0; i < available; ++i) {
            buf[i] = sig[start + i] * win_ptr[i];
        }

        wht::forward_inplace(buf.data(), plan);
        std::memcpy(dst, buf.data(), n * sizeof(float));
    }

    return out;
}

Spectrogram compute_from_frames(const FrameTransform& frames_in) {
    Spectrogram out;
    out.frames = frames_in.frames;
    out.bins = frames_in.bins;
    out.sample_rate = frames_in.sample_rate;
    out.frame_size = frames_in.frame_size;
    out.transform_size = frames_in.transform_size;
    out.hop = frames_in.hop;
    out.power.assign(out.frames * out.bins, 0.0f);
    out.db.assign(out.frames * out.bins, 0.0f);
    out.time_s.resize(out.frames, 0.0f);
    out.pseudo_freq_hz.resize(out.bins, 0.0f);

    float global_max_power = 0.0f;

    const float* coeffs = frames_in.coeffs.data();
    float* power = out.power.data();

    for (std::size_t m = 0; m < out.frames; ++m) {
        out.time_s[m] = static_cast<float>(m * out.hop) / static_cast<float>(out.sample_rate);

        const std::size_t row = m * out.bins;
        for (std::size_t k = 0; k < out.bins; ++k) {
            const float c = coeffs[row + k];
            const float p = c * c;
            power[row + k] = p;
            global_max_power = std::max(global_max_power, p);
        }
    }

    const float ref = std::max(global_max_power, 1e-12f);

    for (std::size_t i = 0; i < out.power.size(); ++i) {
        out.db[i] = 10.0f * safe_log10(out.power[i] / ref);
    }

    const float fs = static_cast<float>(out.sample_rate);
    const float denom = 2.0f * static_cast<float>(out.bins);
    for (std::size_t k = 0; k < out.bins; ++k) {
        out.pseudo_freq_hz[k] = static_cast<float>(k) * fs / denom;
    }

    return out;
}

Spectrogram compute(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type,
    Ordering ordering,
    bool orthonormal
) {
    auto frames = compute_wht_frames(
        signal, sample_rate, frame_size, hop, window_type, ordering, orthonormal
    );
    return compute_from_frames(frames);
}

} // namespace pseudospectrum