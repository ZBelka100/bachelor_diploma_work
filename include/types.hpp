#pragma once

#include <cstddef>
#include <vector>

enum class Ordering {
    Hadamard,
    Sequency,
    Dyadic
};

enum class WindowType {
    Rect,
    Hann,
    SqrtHann
};

struct FrameTransform {
    std::size_t frames = 0;
    std::size_t bins = 0;
    std::size_t frame_size = 0;
    std::size_t transform_size = 0;
    std::size_t hop = 0;
    int sample_rate = 0;

    // row-major: frames x bins
    std::vector<float> coeffs;
};

struct Spectrogram {
    std::size_t frames = 0;
    std::size_t bins = 0;
    int sample_rate = 0;

    std::size_t frame_size = 0;
    std::size_t transform_size = 0;
    std::size_t hop = 0;

    // row-major: frames x bins
    std::vector<float> power;
    std::vector<float> db;
    std::vector<float> time_s;
    std::vector<float> pseudo_freq_hz;
};

struct ReconstructionResult {
    std::vector<float> signal;
    std::vector<float> window_mass;
};

struct ErrorMetrics {
    double mse = 0.0;
    double rmse = 0.0;
    double max_abs = 0.0;
    double snr_db = 0.0;
};