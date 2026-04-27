#include "framing.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

TEST(FramingTest, MakeFramePadsWithZeros) {
    const std::vector<float> x = {1.0f, 2.0f, 3.0f};

    const auto frame = framing::make_frame(x, 0, 5);

    ASSERT_EQ(frame.size(), 5);
    EXPECT_FLOAT_EQ(frame[0], 1.0f);
    EXPECT_FLOAT_EQ(frame[1], 2.0f);
    EXPECT_FLOAT_EQ(frame[2], 3.0f);
    EXPECT_FLOAT_EQ(frame[3], 0.0f);
    EXPECT_FLOAT_EQ(frame[4], 0.0f);
}

TEST(FramingTest, MakeFrameFromOffset) {
    const std::vector<float> x = {1.0f, 2.0f, 3.0f};

    const auto frame = framing::make_frame(x, 2, 3);

    ASSERT_EQ(frame.size(), 3);
    EXPECT_FLOAT_EQ(frame[0], 3.0f);
    EXPECT_FLOAT_EQ(frame[1], 0.0f);
    EXPECT_FLOAT_EQ(frame[2], 0.0f);
}

TEST(FramingTest, CountFrames) {
    EXPECT_EQ(framing::count_frames(0, 4, 2), 0);
    EXPECT_EQ(framing::count_frames(1, 4, 2), 1);
    EXPECT_EQ(framing::count_frames(8, 4, 2), 4);
    EXPECT_EQ(framing::count_frames(9, 4, 2), 5);
}

TEST(FramingTest, CountFramesRejectsInvalidArguments) {
    EXPECT_THROW(framing::count_frames(8, 0, 2), std::invalid_argument);
    EXPECT_THROW(framing::count_frames(8, 4, 0), std::invalid_argument);
}