#include "window.hpp"

#include <cmath>
#include <stdexcept>

namespace window {

std::vector<float> make(WindowType type, std::size_t n) {
    std::vector<float> w(n, 1.0f);
    if (type == WindowType::Rect) return w;

    const double two_pi = 2.0 * M_PI;

    // Periodic Hann
    for (std::size_t i = 0; i < n; ++i) {
        const double hann = 0.5 * (1.0 - std::cos(two_pi * static_cast<double>(i) / static_cast<double>(n)));
        if (type == WindowType::Hann) {
            w[i] = static_cast<float>(hann);
        } else {
            w[i] = static_cast<float>(std::sqrt(hann));
        }
    }

    return w;
}

WindowType parse(const std::string& s) {
    if (s == "rect") return WindowType::Rect;
    if (s == "hann") return WindowType::Hann;
    if (s == "sqrt-hann" || s == "root-hann") return WindowType::SqrtHann;
    throw std::invalid_argument("unknown window: " + s);
}

} // namespace window