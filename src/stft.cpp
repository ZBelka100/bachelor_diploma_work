#include "stft.hpp"

#include "framing.hpp"
#include "window.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

#ifdef USE_FFTW
#include <fftw3.h>
#endif

namespace stft {

static float safe_log10(float x) {
    constexpr float eps = 1e-12f;
    return std::log10(std::max(x, eps));
}

#ifndef USE_FFTW

static bool is_power_of_two(std::size_t n) {
    return n != 0 && ((n & (n - 1)) == 0);
}

static std::size_t next_power_of_two(std::size_t n) {
    if (n == 0) return 1;
    std::size_t p = 1;
    while (p < n) p <<= 1U;
    return p;
}

static void fft_inplace(std::vector<std::complex<float>>& a, bool inverse) {
    const std::size_t n = a.size();
    if (!is_power_of_two(n)) {
        throw std::invalid_argument("fft_inplace: n must be power of two");
    }

    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1U;
        for (; j & bit; bit >>= 1U) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    const float pi = static_cast<float>(M_PI);

    for (std::size_t len = 2; len <= n; len <<= 1U) {
        const float ang = 2.0f * pi / static_cast<float>(len) * (inverse ? 1.0f : -1.0f);
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
        for (auto& x : a) x *= inv_n;
    }
}

#endif

Spectrogram compute(
    const std::vector<float>& signal,
    int sample_rate,
    std::size_t frame_size,
    std::size_t hop,
    WindowType window_type
) {
    if (sample_rate <= 0 || frame_size == 0 || hop == 0) {
        throw std::invalid_argument("stft::compute: invalid parameters");
    }

    const std::size_t frames = framing::count_frames(signal.size(), frame_size, hop);
    const auto win = window::make(window_type, frame_size);

#ifdef USE_FFTW
    const std::size_t nfft = frame_size;
#else
    const std::size_t nfft = next_power_of_two(frame_size);
#endif

    const std::size_t bins = nfft / 2 + 1;

    Spectrogram out;
    out.frames = frames;
    out.bins = bins;
    out.sample_rate = sample_rate;
    out.frame_size = frame_size;
    out.transform_size = nfft;
    out.hop = hop;
    out.power.assign(frames * bins, 0.0f);
    out.db.assign(frames * bins, 0.0f);
    out.time_s.resize(frames, 0.0f);
    out.pseudo_freq_hz.resize(bins, 0.0f);

    float global_max_power = 0.0f;

#ifdef USE_FFTW
    std::vector<float> in(nfft, 0.0f);
    std::vector<fftwf_complex> out_fft(bins);
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(
        static_cast<int>(nfft),
        in.data(),
        out_fft.data(),
        FFTW_MEASURE
    );
    if (!plan) {
        throw std::runtime_error("FFTW: failed to create r2c plan");
    }

    for (std::size_t m = 0; m < frames; ++m) {
        const std::size_t start = m * hop;
        std::fill(in.begin(), in.end(), 0.0f);

        for (std::size_t i = 0; i < frame_size; ++i) {
            const std::size_t pos = start + i;
            if (pos < signal.size()) {
                in[i] = signal[pos] * win[i];
            }
        }

        fftwf_execute(plan);
        out.time_s[m] = static_cast<float>(start) / static_cast<float>(sample_rate);

        for (std::size_t k = 0; k < bins; ++k) {
            const float re = out_fft[k][0];
            const float im = out_fft[k][1];
            const float p = re * re + im * im;
            out.power[m * bins + k] = p;
            global_max_power = std::max(global_max_power, p);
        }
    }

    fftwf_destroy_plan(plan);

#else
    std::vector<std::complex<float>> a(nfft, {0.0f, 0.0f});

    for (std::size_t m = 0; m < frames; ++m) {
        const std::size_t start = m * hop;
        std::fill(a.begin(), a.end(), std::complex<float>(0.0f, 0.0f));

        for (std::size_t i = 0; i < frame_size; ++i) {
            const std::size_t pos = start + i;
            if (pos < signal.size()) {
                a[i] = std::complex<float>(signal[pos] * win[i], 0.0f);
            }
        }

        fft_inplace(a, false);
        out.time_s[m] = static_cast<float>(start) / static_cast<float>(sample_rate);

        for (std::size_t k = 0; k < bins; ++k) {
            const float p = std::norm(a[k]);
            out.power[m * bins + k] = p;
            global_max_power = std::max(global_max_power, p);
        }
    }
#endif

    const float ref = std::max(global_max_power, 1e-12f);

    for (std::size_t i = 0; i < out.power.size(); ++i) {
        out.db[i] = 10.0f * safe_log10(out.power[i] / ref);
    }

    for (std::size_t k = 0; k < bins; ++k) {
        out.pseudo_freq_hz[k] =
            static_cast<float>(k) * static_cast<float>(sample_rate) /
            static_cast<float>(nfft);
    }

    return out;
}

} // namespace stft