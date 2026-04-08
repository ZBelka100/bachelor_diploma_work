#pragma once

#include <string>
#include <vector>

namespace audio_io {

struct AudioBuffer {
    int sample_rate = 0;
    int channels = 0;
    std::vector<float> interleaved;
};

AudioBuffer read_audio(const std::string& path);
std::vector<float> to_mono(const AudioBuffer& buffer);
void write_wav_mono(const std::string& path, const std::vector<float>& mono, int sample_rate);

} // namespace audio_io