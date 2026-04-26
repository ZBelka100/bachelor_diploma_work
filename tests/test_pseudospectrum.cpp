#include "pseudospectrum.hpp"
#include "wht.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <stdexcept>

int main() {
    const size_t N = 256;
    std::vector<float> signal(N);
    for (size_t i = 0; i < N; ++i) {
        signal[i] = static_cast<float>(i) / N;
    }

    const size_t frame_size = 64;
    const size_t hop = 16;

    try {
        auto spec = pseudospectrum::compute(signal, frame_size, hop, Ordering::Sequency, true);
        std::cout << "Pseudospectrum compute (Sequency)\n";
        if (spec.frames == 0) return 1;
        if (spec.power.size() != spec.frames * spec.bins) return 1;
        if (spec.time_s.size() != spec.frames) return 1;
        if (spec.pseudo_freq_hz.size() != spec.bins) return 1;
        for (float p : spec.power) {
            if (p < 0.0f) return 1;
        }
        std::cout << "PASS compute test\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: unexpected exception: " << e.what() << "\n";
        return 1;
    }

    auto frames = pseudospectrum::compute_frames(signal, frame_size, hop, Ordering::Sequency, true);
    auto spec2 = pseudospectrum::compute_from_frames(frames);
    if (spec2.frames != frames.frames) return 1;

    std::vector<float> empty_signal;
    try {
        pseudospectrum::compute(empty_signal, frame_size, hop);
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "PASS\n";
    return 0;
}
