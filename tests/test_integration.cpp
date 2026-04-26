#include "metrics.hpp"
#include "pseudospectrum.hpp"
#include "reconstruction.hpp"
#include "stft.hpp"
#include "types.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const int sample_rate = 16000;
    const std::size_t n = 4096;

    std::vector<float> signal(n, 0.0f);

    for (std::size_t i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        signal[i] = 0.5f * std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * t);
    }

    const std::size_t frame_size = 512;
    const std::size_t hop = 256;
    const auto window_type = WindowType::SqrtHann;

    auto wht_frames = pseudospectrum::compute_wht_frames(
        signal,
        sample_rate,
        frame_size,
        hop,
        window_type,
        Ordering::Sequency,
        true
    );

    auto wht_spec = pseudospectrum::compute_from_frames(wht_frames);

    assert(wht_spec.frames == wht_frames.frames);
    assert(wht_spec.bins == wht_frames.bins);
    assert(!wht_spec.db.empty());

    auto wht_rec = reconstruction::from_wht_frames(
        wht_frames,
        Ordering::Sequency,
        window_type,
        true
    );

    const std::size_t common = std::min(signal.size(), wht_rec.signal.size());
    std::vector<float> ref(signal.begin(), signal.begin() + common);
    std::vector<float> rec(wht_rec.signal.begin(), wht_rec.signal.begin() + common);

    auto err = metrics::compare(ref, rec);

    assert(err.rmse < 1e-3);
    assert(err.snr_db > 40.0);

    auto stft_spec = stft::compute(
        signal,
        sample_rate,
        frame_size,
        hop,
        window_type
    );

    assert(stft_spec.frames > 0);
    assert(stft_spec.bins > 0);
    assert(!stft_spec.db.empty());

    std::cout << "test_integration passed\n";
    return 0;
}
