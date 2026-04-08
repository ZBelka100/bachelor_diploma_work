#include "audio_io.hpp"

#include <sndfile.h>

#include <stdexcept>
#include <string>

namespace audio_io {

AudioBuffer read_audio(const std::string& path) {
    SF_INFO info{};
    SNDFILE* f = sf_open(path.c_str(), SFM_READ, &info);
    if (!f) {
        throw std::runtime_error(std::string("sf_open failed: ") + sf_strerror(nullptr));
    }

    AudioBuffer out;
    out.sample_rate = info.samplerate;
    out.channels = info.channels;
    out.interleaved.resize(
        static_cast<std::size_t>(info.frames) * static_cast<std::size_t>(info.channels)
    );

    const sf_count_t got = sf_readf_float(f, out.interleaved.data(), info.frames);
    sf_close(f);

    if (got != info.frames) {
        throw std::runtime_error("sf_readf_float: could not read all frames");
    }

    return out;
}

std::vector<float> to_mono(const AudioBuffer& buffer) {
    if (buffer.channels <= 0) return {};

    const std::size_t frames =
        buffer.interleaved.size() / static_cast<std::size_t>(buffer.channels);

    std::vector<float> mono(frames, 0.0f);
    for (std::size_t i = 0; i < frames; ++i) {
        double sum = 0.0;
        for (int ch = 0; ch < buffer.channels; ++ch) {
            sum += buffer.interleaved[i * static_cast<std::size_t>(buffer.channels) + static_cast<std::size_t>(ch)];
        }
        mono[i] = static_cast<float>(sum / static_cast<double>(buffer.channels));
    }

    return mono;
}

void write_wav_mono(const std::string& path, const std::vector<float>& mono, int sample_rate) {
    SF_INFO info{};
    info.channels = 1;
    info.samplerate = sample_rate;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* f = sf_open(path.c_str(), SFM_WRITE, &info);
    if (!f) {
        throw std::runtime_error(std::string("sf_open(write) failed: ") + sf_strerror(nullptr));
    }

    const sf_count_t frames = static_cast<sf_count_t>(mono.size());
    const sf_count_t wrote = sf_writef_float(f, mono.data(), frames);
    sf_close(f);

    if (wrote != frames) {
        throw std::runtime_error("sf_writef_float: could not write all frames");
    }
}

} // namespace audio_io