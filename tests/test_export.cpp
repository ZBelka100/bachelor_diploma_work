#include "export.hpp"
#include "types.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

int main() {
    Spectrogram spec;
    spec.frames = 2;
    spec.bins = 3;
    spec.sample_rate = 16000;
    spec.frame_size = 4;
    spec.transform_size = 4;
    spec.hop = 2;
    spec.power = {
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    };
    spec.db = {
        0.0f, -1.0f, -2.0f,
        -3.0f, -4.0f, -5.0f
    };
    spec.time_s = {0.0f, 0.1f};
    spec.pseudo_freq_hz = {0.0f, 1000.0f, 2000.0f};

    const std::string path = "test_export_tmp.csv";

    export_data::write_spectrogram_csv(path, spec);

    assert(std::filesystem::exists(path));
    assert(std::filesystem::file_size(path) > 0);

    std::filesystem::remove(path);

    std::cout << "test_export passed\n";
    return 0;
}
