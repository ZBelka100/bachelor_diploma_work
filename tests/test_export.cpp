#include "export.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

TEST(ExportTest, WritesVectorCsv) {
    const std::string path = "test_export_vector.csv";

    export_io::write_vector_csv(path, std::vector<float>{1.0f, 2.0f, 3.0f});

    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_GT(std::filesystem::file_size(path), 0U);

    std::filesystem::remove(path);
}

TEST(ExportTest, WritesMatrixCsv) {
    const std::string path = "test_export_matrix.csv";

    export_io::write_matrix_csv(path, std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, 2, 2);

    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_GT(std::filesystem::file_size(path), 0U);

    std::filesystem::remove(path);
}

TEST(ExportTest, WritesSpectrogramBundle) {
    const std::string prefix = "test_export_spec";

    Spectrogram spec;
    spec.frames = 2;
    spec.bins = 2;
    spec.sample_rate = 8000;
    spec.frame_size = 4;
    spec.transform_size = 4;
    spec.hop = 2;
    spec.power = {1.0f, 2.0f, 3.0f, 4.0f};
    spec.db = {0.0f, -1.0f, -2.0f, -3.0f};
    spec.time_s = {0.0f, 0.1f};
    spec.pseudo_freq_hz = {0.0f, 1000.0f};

    export_io::write_spectrogram_csv_bundle(prefix, spec);

    bool found = false;

    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        const auto name = entry.path().filename().string();
        if (name.rfind(prefix, 0) == 0) {
            found = true;
            std::filesystem::remove(entry.path());
        }
    }

    EXPECT_TRUE(found);
}

TEST(ExportTest, WritesFrameTransformBundle) {
    const std::string prefix = "test_export_frames";

    FrameTransform frames;
    frames.frames = 2;
    frames.bins = 2;
    frames.sample_rate = 8000;
    frames.frame_size = 4;
    frames.transform_size = 4;
    frames.hop = 2;
    frames.coeffs = {1.0f, 2.0f, 3.0f, 4.0f};

    export_io::write_frame_transform_csv_bundle(prefix, frames);

    bool found = false;

    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        const auto name = entry.path().filename().string();
        if (name.rfind(prefix, 0) == 0) {
            found = true;
            std::filesystem::remove(entry.path());
        }
    }

    EXPECT_TRUE(found);
}