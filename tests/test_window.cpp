#include "window.hpp"
#include "types.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    {
        auto w = window::make(WindowType::Rect, 8);
        assert(w.size() == 8);

        for (float v : w) {
            assert(std::abs(v - 1.0f) < 1e-6f);
        }
    }

    {
        auto w = window::make(WindowType::Hann, 8);
        assert(w.size() == 8);
        assert(std::abs(w[0]) < 1e-6f);
    }

    {
        auto w = window::make(WindowType::SqrtHann, 8);
        assert(w.size() == 8);
        assert(std::abs(w[0]) < 1e-6f);
    }

    assert(window::parse("rect") == WindowType::Rect);
    assert(window::parse("hann") == WindowType::Hann);
    assert(window::parse("sqrt-hann") == WindowType::SqrtHann);
    assert(window::parse("root-hann") == WindowType::SqrtHann);

    bool thrown = false;
    try {
        (void)window::parse("unknown");
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    assert(thrown);

    std::cout << "test_window passed\n";
    return 0;
}
