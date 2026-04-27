#include "audio_io.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

TEST(AudioIoTest, WritesWavMonoFile) {
    const std::vector<float> signal = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f};
    const std::string path = "test_audio_io_tmp.wav";

    audio_io::write_wav_mono(path, signal, 16000);

    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_GT(std::filesystem::file_size(path), 0U);

    std::filesystem::remove(path);
}