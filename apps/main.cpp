#include "audio_io.hpp"
#include "export.hpp"
#include "metrics.hpp"
#include "pseudospectrum.hpp"
#include "reconstruction.hpp"
#include "stft.hpp"
#include "types.hpp"
#include "window.hpp"

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Args {
    std::string input_path;
    std::string out_prefix = "out";
    std::string recon_path = "out_recon.wav";
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

        const auto wht_frames = pseudospectrum::compute_wht_frames(
            mono,
            audio.sample_rate,
            args.frame,
            args.hop,
            args.window,
            args.ordering,
            true
        );

        const auto wht_spec = pseudospectrum::compute_from_frames(wht_frames);

        export_io::write_frame_transform_csv_bundle(args.out_prefix, wht_frames);
        export_io::write_spectrogram_csv_bundle(args.out_prefix, wht_spec);

        const auto recon = reconstruction::from_wht_frames(
            wht_frames,
            args.ordering,
            args.window,
            true
        );

        const std::size_t common = std::min(mono.size(), recon.signal.size());
        std::vector<float> ref(mono.begin(), mono.begin() + static_cast<std::ptrdiff_t>(common));
        std::vector<float> rec(recon.signal.begin(), recon.signal.begin() + static_cast<std::ptrdiff_t>(common));

        const auto metrics = metrics::compare(ref, rec);
        audio_io::write_wav_mono(args.recon_path, rec, audio.sample_rate);

        std::cout << "\nWHT reconstruction metrics:\n";
        std::cout << "  MSE:     " << metrics.mse << "\n";
        std::cout << "  RMSE:    " << metrics.rmse << "\n";
        std::cout << "  MaxAbs:  " << metrics.max_abs << "\n";
        std::cout << "  SNR dB:  " << metrics.snr_db << "\n";

        if (args.do_stft) {
            const auto stft_spec = stft::compute(
                mono,
                audio.sample_rate,
                args.frame,
                args.hop,
                WindowType::Hann
            );

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

            std::cout << "\nSTFT exported too.\n";
        }

        std::cout << "\nDone.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}