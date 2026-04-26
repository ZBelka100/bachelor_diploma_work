#include "reconstruction.hpp"
#include "wht.hpp"
#include "metrics.hpp"
#include "window.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

int main() {
    const size_t N = 256;
    std::vector<float> signal(N);
    for (size_t i = 0; i < N; ++i) {
        signal[i] = std::sin(2.0f * 3.14159265358979323846f * i / 50.0f);
    }

    const size_t frame_size = 64;
    const size_t hop = 16;

    try {
        auto frames = reconstruction::compute_frames(signal, frame_size, hop, Ordering::Sequency, true, WindowType::Hann);
        auto reconstructed = reconstruction::overlap_add(frames, N);
        auto m = metrics::compare(signal, reconstructed);
        std::cout << "Reconstruction error: MSE=" << m.mse << ", SNR=" << m.snr_db << " dB\n";
        if (m.snr_db < 40.0) return 1;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: unexpected exception: " << e.what() << "\n";
        return 1;
    }

    std::vector<float> empty_signal;
    try {
        reconstruction::compute_frames(empty_signal, frame_size, hop);
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "PASS\n";
    return 0;
}
