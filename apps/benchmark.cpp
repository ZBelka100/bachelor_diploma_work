#include "audio_io.hpp"
#include "bench_external.hpp"
#include "pseudospectrum.hpp"
#include "stft.hpp"
#include "wht.hpp"
#include "window.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;

struct Args {
    std::string input_path;
    std::string csv_path = "benchmark_results.csv";
    std::size_t frame = 1024;
    std::size_t hop = 256;
    std::size_t iterations = 100;
    WindowType window = WindowType::SqrtHann;
    Ordering ordering = Ordering::Sequency;
};

std::string need_value(int& i, int argc, char** argv, const std::string& name) {
    if (i + 1 >= argc) {
        throw std::invalid_argument("Expected value after " + name);
    }
    return argv[++i];
}

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
        if (arg == "--input") a.input_path = need_value(i, argc, argv, arg);
        else if (arg == "--csv") a.csv_path = need_value(i, argc, argv, arg);
        else if (arg == "--frame") a.frame = static_cast<std::size_t>(std::stoul(need_value(i, argc, argv, arg)));
        else if (arg == "--hop") a.hop = static_cast<std::size_t>(std::stoul(need_value(i, argc, argv, arg)));
        else if (arg == "--iters") a.iterations = static_cast<std::size_t>(std::stoul(need_value(i, argc, argv, arg)));
        else if (arg == "--window") a.window = window::parse(need_value(i, argc, argv, arg));
        else if (arg == "--order") a.ordering = parse_ordering(need_value(i, argc, argv, arg));
        else throw std::invalid_argument("Unknown argument: " + arg);
    }

    if (a.input_path.empty()) {
        throw std::invalid_argument("Missing --input");
    }
    if (a.frame == 0 || a.hop == 0 || a.iterations == 0) {
        throw std::invalid_argument("frame, hop and iterations must be > 0");
    }
    return a;
}

double ms_between(const Clock::time_point& t0, const Clock::time_point& t1) {
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
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
    }
    return "unknown";
}

void append_csv_row(
    const std::string& path,
    const std::string& method,
    const std::string& mode,
    std::size_t frame_size,
    std::size_t hop,
    const std::string& window_name,
    const std::string& ordering_name,
    std::size_t signal_len,
    std::size_t iterations,
    double total_ms,
    double avg_ms,
    double throughput_frames_per_sec,
    double throughput_samples_per_sec)
{
    bool write_header = false;
    {
        std::ifstream in(path);
        write_header = !in.good() || in.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream out(path, std::ios::app);
    if (!out) {
        throw std::runtime_error("Cannot open CSV for writing: " + path);
    }

    if (write_header) {
        out << "method,mode,frame_size,hop,window,ordering,signal_len,iterations,total_ms,avg_ms,throughput_frames_per_sec,throughput_samples_per_sec\n";
    }

    out << method << ','
        << mode << ','
        << frame_size << ','
        << hop << ','
        << window_name << ','
        << ordering_name << ','
        << signal_len << ','
        << iterations << ','
        << std::fixed << std::setprecision(6)
        << total_ms << ','
        << avg_ms << ','
        << throughput_frames_per_sec << ','
        << throughput_samples_per_sec
        << '\n';
}
} // namespace

int main(int argc, char** argv) {
    try {
        const Args args = parse_args(argc, argv);

        const auto audio = audio_io::read_audio(args.input_path);
        const auto mono = audio_io::to_mono(audio);

        const std::size_t n = wht::next_power_of_two(args.frame);
        std::vector<float> frame_buf(n, 0.0f);
        const std::size_t limit = std::min(args.frame, mono.size());
        for (std::size_t i = 0; i < limit; ++i) {
            frame_buf[i] = mono[i];
        }

        // ---- Internal FWHT single-frame benchmark ----
        {
            wht::Plan plan(n, args.ordering, true);

            for (int i = 0; i < 10; ++i) {
                auto tmp = frame_buf;
                wht::forward_inplace(tmp.data(), plan);
            }

            const auto t0 = Clock::now();
            for (std::size_t i = 0; i < args.iterations; ++i) {
                auto tmp = frame_buf;
                wht::forward_inplace(tmp.data(), plan);
            }
            const auto t1 = Clock::now();

            const double total_ms = ms_between(t0, t1);
            const double avg_ms = total_ms / static_cast<double>(args.iterations);
            const double fps = 1000.0 / avg_ms;
            const double sps = fps * static_cast<double>(args.frame);

            append_csv_row(
                args.csv_path,
                "fwht",
                "single_frame",
                args.frame,
                args.hop,
                to_string(args.window),
                to_string(args.ordering),
                mono.size(),
                args.iterations,
                total_ms,
                avg_ms,
                fps,
                sps
            );
        }

        // ---- Library FFHT single-frame benchmark ----
        {
            const double avg_ms =
                bench_external::run_ffht_single_frame_ms(frame_buf, args.iterations);
            const double total_ms = avg_ms * static_cast<double>(args.iterations);
            const double fps = 1000.0 / avg_ms;
            const double sps = fps * static_cast<double>(args.frame);

            append_csv_row(
                args.csv_path,
                "fwht_ffht",
                "single_frame",
                args.frame,
                args.hop,
                to_string(args.window),
                to_string(args.ordering),
                mono.size(),
                args.iterations,
                total_ms,
                avg_ms,
                fps,
                sps
            );
        }

        // ---- Library FFTW single-frame benchmark ----
        {
            const double avg_ms =
                bench_external::run_fftw_r2c_single_frame_ms(frame_buf, args.iterations);
            const double total_ms = avg_ms * static_cast<double>(args.iterations);
            const double fps = 1000.0 / avg_ms;
            const double sps = fps * static_cast<double>(args.frame);

            append_csv_row(
                args.csv_path,
                "fft_fftw",
                "single_frame",
                args.frame,
                args.hop,
                to_string(args.window),
                "na",
                mono.size(),
                args.iterations,
                total_ms,
                avg_ms,
                fps,
                sps
            );
        }

        // ---- WHT pipeline benchmark ----
        {
            std::size_t frames_count = 0;

            for (int i = 0; i < 3; ++i) {
                auto frames = pseudospectrum::compute_wht_frames(
                    mono, audio.sample_rate, args.frame, args.hop,
                    args.window, args.ordering, true);
                frames_count = frames.frames;
            }

            const auto t0 = Clock::now();
            for (std::size_t i = 0; i < args.iterations; ++i) {
                auto frames = pseudospectrum::compute_wht_frames(
                    mono, audio.sample_rate, args.frame, args.hop,
                    args.window, args.ordering, true);
                frames_count = frames.frames;
            }
            const auto t1 = Clock::now();

            const double total_ms = ms_between(t0, t1);
            const double avg_ms = total_ms / static_cast<double>(args.iterations);
            const double fps = (frames_count * args.iterations) / (total_ms / 1000.0);
            const double sps = (mono.size() * args.iterations) / (total_ms / 1000.0);

            append_csv_row(
                args.csv_path,
                "wht_pipeline",
                "full_signal",
                args.frame,
                args.hop,
                to_string(args.window),
                to_string(args.ordering),
                mono.size(),
                args.iterations,
                total_ms,
                avg_ms,
                fps,
                sps
            );
        }

        // ---- STFT pipeline benchmark ----
        {
            std::size_t frames_count = 0;

            for (int i = 0; i < 3; ++i) {
                auto spec = stft::compute(
                    mono, audio.sample_rate, args.frame, args.hop, WindowType::Hann);
                frames_count = spec.frames;
            }

            const auto t0 = Clock::now();
            for (std::size_t i = 0; i < args.iterations; ++i) {
                auto spec = stft::compute(
                    mono, audio.sample_rate, args.frame, args.hop, WindowType::Hann);
                frames_count = spec.frames;
            }
            const auto t1 = Clock::now();

            const double total_ms = ms_between(t0, t1);
            const double avg_ms = total_ms / static_cast<double>(args.iterations);
            const double fps = (frames_count * args.iterations) / (total_ms / 1000.0);
            const double sps = (mono.size() * args.iterations) / (total_ms / 1000.0);

            append_csv_row(
                args.csv_path,
                "stft_pipeline",
                "full_signal",
                args.frame,
                args.hop,
                "hann",
                "na",
                mono.size(),
                args.iterations,
                total_ms,
                avg_ms,
                fps,
                sps
            );
        }

        std::cout << "Benchmark complete: " << args.csv_path << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << '\n';
        return 1;
    }
}