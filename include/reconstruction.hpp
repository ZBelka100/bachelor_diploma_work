#pragma once

#include "stft.hpp"
#include "types.hpp"

namespace reconstruction {

ReconstructionResult from_wht_frames(
    const FrameTransform& frames,
    Ordering ordering,
    WindowType synthesis_window,
    bool orthonormal
);

ReconstructionResult from_stft_frames(
    const stft::ComplexFrameTransform& frames,
    WindowType synthesis_window
);

} // namespace reconstruction