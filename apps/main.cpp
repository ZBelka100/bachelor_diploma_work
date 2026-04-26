#include "audio_io.hpp"
#include "export.hpp"
#include "metrics.hpp"
#include "pseudospectrum.hpp"
#include "reconstruction.hpp"
#include "stft.hpp"
#include "types.hpp"
#include "window.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Args {
    std::string input_path;
    std::string out_prefix = "out";
    std::string recon_path = "out_recon.wav";
    std::string stft_recon_path = "out_stft_recon.wav";
    std::string summary_csv;
    std::size_t frame = 1024;
    std::size_t hop = 256;
    WindowType window = WindowType::SqrtHann;
    Ordering ordering = Ordering::Sequency;
    bool do_stft = true;
};

Ordering parse_ordering(const std::string& s) {
    if (s == "hadamard") return Ordering::Hadamard;
    if (s == "sequency") return Ordering::Sequency;
    if (s == "dyadic") return Ordering::Dyadic;
    throw std::invalid_argument("Unknown ordering: " + s);
}

std::string to_string(WindowType w) {
    switch (w) {
        case WindowType::Rect: return "rect";
        case WindowType::Hann: return "hann";
        case WindowType::SqrtHann: return "sqrt-hann";
    }
    return "unknown";
}

std::string to_string(Ordering o) {
    switch (o) {
        case Ordering::Hadamard: return "hadamard";
        case Ordering::Sequency: return "sequency";
        case Ordering::Dyadic: return "dyadic";
        default: return "unknown";
    }
}

Args parse_args(int argc, char** argv) {
    Args a;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto need_value = [&](const std::string& name) -> std::string {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Expected value after " + name);
            }
            return argv[++i];
        };

        if (arg == "--input") {
            a.input_path = need_value(arg);
        } else if (arg == "--out") {
            a.out_prefix = need_value(arg);
        } else if (arg == "--reconstruct") {
            a.recon_path = need_value(arg);
        } else if (arg == "--stft-reconstruct") {
            a.stft_recon_path = need_value(arg);
        } else if (arg == "--summary-csv") {
            a.summary_csv = need_value(arg);
        } else if (arg == "--frame") {
            a.frame = static_cast<std::size_t>(std::stoul(need_value(arg)));
        } else if (arg == "--hop") {
            a.hop = static_cast<std::size_t>(std::stoul(need_value(arg)));
        } else if (arg == "--window") {
            a.window = window::parse(need_value(arg));
        } else if (arg == "--order") {
            a.ordering = parse_ordering(need_value(arg));
        } else if (arg == "--do-stft") {
            a.do_stft = (std::stoi(need_value(arg)) != 0);
        } else {
            throw std::invalid_argument("Unknown argument: " + arg);
        }
    }

    if (a.input_path.empty()) {
        throw std::invalid_argument("Missing --input path");
    }

    return a;
}

void append_summary_row(
    const std::string& path,
    const std::string& input_path,
    const std::string& method,
    const Args& args,
    const ErrorMetrics& m,
    double transform_ms,
    double recon_ms
) {
    const bool write_header = [&]() {
        std::ifstream in(path);
        return !in.good() || in.peek() == std::ifstream::traits_type::eof();
    }();

    std::ofstream out(path, std::ios::app);
    if (!out) {
        throw std::runtime_error("Cannot open summary csv: " + path);
    }

    if (write_header) {
        out << "input,method,window,ordering,frame,hop,"
               "mse,rmse,max_abs,snr_db,"
               "transform_ms,recon_ms,total_ms\n";
    }

    out << std::quoted(input_path) << ","
        << method << ","
        << to_string(args.window) << ","
        << to_string(args.ordering) << ","
        << args.frame << ","
        << args.hop << ","
        << m.mse << ","
        << m.rmse << ","
        << m.max_abs << ","
        << m.snr_db << ","
        << transform_ms << ","
        << recon_ms << ","
        << (transform_ms + recon_ms) << "\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Args args = parse_args(argc, argv);

        const auto audio = audio_io::read_audio(args.input_path);
        const auto mono = audio_io::to_mono(audio);

        std::cout << "Loaded audio:\n";
        std::cout << "  sample rate: " << audio.sample_rate << "\n";
        std::cout << "  channels:    " << audio.channels << "\n";
        std::cout << "  samples:     " << mono.size() << "\n";

        // ---------------- WHT ----------------
        const auto wht_t0 = std::chrono::steady_clock::now();
        const auto wht_frames = pseudospectrum::compute_wht_frames(
            mono,
            audio.sample_rate,
            args.frame,
            args.hop,
            args.window,
            args.ordering,
            true
        );
        const auto wht_t1 = std::chrono::steady_clock::now();

        const auto wht_spec = pseudospectrum::compute_from_frames(wht_frames);

        export_io::write_frame_transform_csv_bundle(args.out_prefix, wht_frames);
        export_io::write_spectrogram_csv_bundle(args.out_prefix, wht_spec);

        const auto wht_r0 = std::chrono::steady_clock::now();
        const auto recon = reconstruction::from_wht_frames(
            wht_frames,
            args.ordering,
            args.window,
            true
        );
        const auto wht_r1 = std::chrono::steady_clock::now();

        const std::size_t common = std::min(mono.size(), recon.signal.size());
        std::vector<float> ref(
            mono.begin(),
            mono.begin() + static_cast<std::ptrdiff_t>(common)
        );
        std::vector<float> rec(
            recon.signal.begin(),
            recon.signal.begin() + static_cast<std::ptrdiff_t>(common)
        );

        const auto wht_metrics = metrics::compare(ref, rec);
        audio_io::write_wav_mono(args.recon_path, rec, audio.sample_rate);

        const double wht_transform_ms =
            std::chrono::duration<double, std::milli>(wht_t1 - wht_t0).count();
        const double wht_recon_ms =
            std::chrono::duration<double, std::milli>(wht_r1 - wht_r0).count();

        std::cout << "\nWHT reconstruction metrics:\n";
        std::cout << "  MSE:           " << wht_metrics.mse << "\n";
        std::cout << "  RMSE:          " << wht_metrics.rmse << "\n";
        std::cout << "  MaxAbs:        " << wht_metrics.max_abs << "\n";
        std::cout << "  SNR dB:        " << wht_metrics.snr_db << "\n";
        std::cout << "  Transform ms:  " << wht_transform_ms << "\n";
        std::cout << "  Recon ms:      " << wht_recon_ms << "\n";
        std::cout << "  Total ms:      " << (wht_transform_ms + wht_recon_ms) << "\n";

        if (!args.summary_csv.empty()) {
            append_summary_row(
                args.summary_csv,
                args.input_path,
                "wht",
                args,
                wht_metrics,
                wht_transform_ms,
                wht_recon_ms
            );
        }

        // ---------------- STFT ----------------
        if (args.do_stft) {
            const auto stft_t0 = std::chrono::steady_clock::now();
            const auto stft_frames = stft::compute_frames(
                mono,
                audio.sample_rate,
                args.frame,
                args.hop,
                args.window
            );
            const auto stft_t1 = std::chrono::steady_clock::now();

            const auto stft_spec = stft::compute_from_frames(stft_frames);

            export_io::write_matrix_csv(
                args.out_prefix + "_stft_power.csv",
                stft_spec.power,
                stft_spec.frames,
                stft_spec.bins
            );

            export_io::write_matrix_csv(
                args.out_prefix + "_stft_db.csv",
                stft_spec.db,
                stft_spec.frames,
                stft_spec.bins
            );

            export_io::write_vector_csv(
                args.out_prefix + "_stft_time_s.csv",
                stft_spec.time_s
            );

            export_io::write_vector_csv(
                args.out_prefix + "_freq_hz.csv",
                stft_spec.pseudo_freq_hz
            );

            const auto stft_r0 = std::chrono::steady_clock::now();
            const auto stft_recon = reconstruction::from_stft_frames(
                stft_frames,
                args.window
            );
            const auto stft_r1 = std::chrono::steady_clock::now();

            const std::size_t stft_common =
                std::min(mono.size(), stft_recon.signal.size());

            std::vector<float> stft_ref(
                mono.begin(),
                mono.begin() + static_cast<std::ptrdiff_t>(stft_common)
            );
            std::vector<float> stft_rec(
                stft_recon.signal.begin(),
                stft_recon.signal.begin() + static_cast<std::ptrdiff_t>(stft_common)
            );

            const auto stft_metrics = metrics::compare(stft_ref, stft_rec);
            audio_io::write_wav_mono(args.stft_recon_path, stft_rec, audio.sample_rate);

            const double stft_transform_ms =
                std::chrono::duration<double, std::milli>(stft_t1 - stft_t0).count();
            const double stft_recon_ms =
                std::chrono::duration<double, std::milli>(stft_r1 - stft_r0).count();

            std::cout << "\nSTFT reconstruction metrics:\n";
            std::cout << "  MSE:           " << stft_metrics.mse << "\n";
            std::cout << "  RMSE:          " << stft_metrics.rmse << "\n";
            std::cout << "  MaxAbs:        " << stft_metrics.max_abs << "\n";
            std::cout << "  SNR dB:        " << stft_metrics.snr_db << "\n";
            std::cout << "  Transform ms:  " << stft_transform_ms << "\n";
            std::cout << "  Recon ms:      " << stft_recon_ms << "\n";
            std::cout << "  Total ms:      " << (stft_transform_ms + stft_recon_ms) << "\n";

            if (!args.summary_csv.empty()) {
                append_summary_row(
                    args.summary_csv,
                    args.input_path,
                    "stft",
                    args,
                    stft_metrics,
                    stft_transform_ms,
                    stft_recon_ms
                );
            }

            std::cout << "\nSTFT exported too.\n";
        }

        std::cout << "\nDone.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}