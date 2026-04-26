#include "audio_io.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    std::vector<float> x = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f};

    const std::string path = "test_audio_io_tmp.wav";

    audio_io::write_wav_mono(path, x, 16000);

    const auto y = audio_io::read_wav_mono(path);

    assert(y.sample_rate == 16000);
    assert(y.samples.size() == x.size());

    for (std::size_t i = 0; i < x.size(); ++i) {
        assert(std::abs(y.samples[i] - x[i]) < 1e-3f);
    }

    std::cout << "test_audio_io passed\n";
    return 0;
}
