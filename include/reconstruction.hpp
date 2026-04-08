#pragma once

#include "types.hpp"

#include <cstddef>
#include <vector>

namespace reconstruction {

ReconstructionResult from_wht_frames(
    const FrameTransform& frames,
    Ordering ordering = Ordering::Sequency,
    WindowType synthesis_window = WindowType::SqrtHann,
    bool orthonormal = true
);

} // namespace reconstruction