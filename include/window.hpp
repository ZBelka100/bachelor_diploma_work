#pragma once

#include "types.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace window {

std::vector<float> make(WindowType type, std::size_t n);
WindowType parse(const std::string& s);

} // namespace window