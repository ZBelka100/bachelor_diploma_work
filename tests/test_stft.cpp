#include "stft.hpp"
#include "metrics.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

int main() {
    const int sample_rate = 44100;
    const float freq = 440.0f;
    const size_t num_samples = sample_rate;
    std::vector<float> signal(num_samples);
    for (size_t i = 0; i < num_samples; ++i) {
        signal[i] = std::sin(2.0f * 3.14159265358979323846f * freq * i / sample_rate);
    }

    const size_t frame_size = 1024;
    const size_t hop = 256;

    try {
        auto spec = stft::compute(signal, sample_rate, frame_size, hop, WindowType::Hann);
        std::cout << "STFT compute test\n";
        std::cout << "Frames: " << spec.frames << ", Bins: " << spec.bins << "\n";
        if (spec.frames == 0) {
            std::cerr << "FAIL: zero frames\n";
            return 1;
        }
        if (spec.bins != frame_size / 2 + 1) {
            std::cerr << "FAIL: bins mismatch (" << spec.bins << " vs " << (frame_size / 2 + 1) << ")\n";
            return 1;
        }
        if (spec.power.size() != spec.frames * spec.bins) {
            std::cerr << "FAIL: power size mismatch\n";
            return 1;
        }
        for (size_t i = 0; i < spec.bins; ++i) {
            if (spec.pseudo_freq_hz[i] < 0.0f) {
                std::cerr << "FAIL: negative frequency\n";
                return 1;
            }
        }
        if (spec.time_s.size() != spec.frames) {
            std::cerr << "FAIL: time vector size mismatch\n";
            return 1;
        }
        for (size_t i = 1; i < spec.time_s.size(); ++i) {
            if (spec.time_s[i] <= spec.time_s[i-1]) {
                std::cerr << "FAIL: time vector not monotonic\n";
                return 1;
            }
        }
        std::cout << "PASS basic test\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: unexpected exception: " << e.what() << "\n";
        return 1;
    }

    try {
        auto frames = stft::compute_frames(signal, sample_rate, frame_size, hop, WindowType::Hann);
        auto spec = stft::compute_from_frames(frames);
        std::cout << "STFT compute_from_frames test\n";
        if (spec.frames != frames.frames) {
            std::cerr << "FAIL: frames mismatch\n";
            return 1;
        }
        std::cout << "PASS compute_from_frames test\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: unexpected exception: " << e.what() << "\n";
        return 1;
    }

    std::vector<float> empty_signal;
    try {
        stft::compute(empty_signal, sample_rate, frame_size, hop);
        std::cerr << "FAIL: expected exception for empty signal\n";
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "PASS\n";
    return 0;
}
