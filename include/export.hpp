#pragma once

#include "types.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace export_io {

void write_vector_csv(const std::string& path, const std::vector<float>& v);
void write_matrix_csv(
    const std::string& path,
    const std::vector<float>& a,
    std::size_t rows,
    std::size_t cols
);

void write_spectrogram_csv_bundle(const std::string& prefix, const Spectrogram& spec);
void write_frame_transform_csv_bundle(const std::string& prefix, const FrameTransform& frames);

} // namespace export_io