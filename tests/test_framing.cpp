#include "framing.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <stdexcept>

int main() {
    std::vector<float> x = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};

    auto frame = framing::make_frame(x, 0, 5);
    std::cout << "make_frame full\n";
    if (frame.size() != 5) return 1;
    for (size_t i = 0; i < 5; ++i) {
        if (frame[i] != x[i]) return 1;
    }

    auto frame2 = framing::make_frame(x, 3, 5);
    std::cout << "make_frame with zero-padding\n";
    if (frame2.size() != 5) return 1;
    if (frame2[0] != 4.0f || frame2[1] != 5.0f) return 1;
    if (frame2[2] != 0.0f || frame2[3] != 0.0f || frame2[4] != 0.0f) return 1;

    auto frame3 = framing::make_frame(x, 10, 5);
    std::cout << "make_frame out of bounds\n";
    if (frame3.size() != 5) return 1;
    for (size_t i = 0; i < 5; ++i) {
        if (frame3[i] != 0.0f) return 1;
    }

    size_t frames = framing::count_frames(100, 10, 5);
    std::cout << "count_frames(100,10,5) = " << frames << "\n";
    if (frames != 20) return 1;

    frames = framing::count_frames(100, 10, 7);
    std::cout << "count_frames(100,10,7) = " << frames << "\n";
    if (frames != 15) return 1;

    try {
        framing::count_frames(100, 0, 5);
        return 1;
    } catch (const std::invalid_argument&) {
    }

    std::cout << "PASS\n";
    return 0;
}
