#include "export.hpp"

#include <fstream>
#include <stdexcept>

namespace export_io {

void write_vector_csv(const std::string& path, const std::vector<float>& v) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }

    for (std::size_t i = 0; i < v.size(); ++i) {
        out << v[i];
        if (i + 1 != v.size()) out << "\n";
    }
}

void write_matrix_csv(
    const std::string& path,
    const std::vector<float>& a,
    std::size_t rows,
    std::size_t cols
) {
    if (a.size() != rows * cols) {
        throw std::invalid_argument("write_matrix_csv: size mismatch");
    }

    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }

    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            out << a[r * cols + c];
            if (c + 1 != cols) out << ",";
        }
        if (r + 1 != rows) out << "\n";
    }
}

void write_spectrogram_csv_bundle(const std::string& prefix, const Spectrogram& spec) {
    write_matrix_csv(prefix + "_wht_power.csv", spec.power, spec.frames, spec.bins);
    write_matrix_csv(prefix + "_wht_db.csv", spec.db, spec.frames, spec.bins);
    write_vector_csv(prefix + "_time_s.csv", spec.time_s);
    write_vector_csv(prefix + "_pseudofreq_hz.csv", spec.pseudo_freq_hz);
}

void write_frame_transform_csv_bundle(const std::string& prefix, const FrameTransform& frames) {
    write_matrix_csv(prefix + "_wht_coeffs.csv", frames.coeffs, frames.frames, frames.bins);
}

} // namespace export_io