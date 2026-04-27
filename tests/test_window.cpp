#include "window.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

TEST(WindowTest, RectWindowContainsOnes) {
    const auto w = window::make(WindowType::Rect, 8);

    ASSERT_EQ(w.size(), 8);

    for (float value : w) {
        EXPECT_NEAR(value, 1.0f, 1e-6f);
    }
}

TEST(WindowTest, HannWindowHasExpectedShape) {
    const auto w = window::make(WindowType::Hann, 8);

    ASSERT_EQ(w.size(), 8);
    EXPECT_NEAR(w[0], 0.0f, 1e-6f);
    EXPECT_GT(w[4], 0.99f);
}

TEST(WindowTest, SqrtHannWindowHasExpectedShape) {
    const auto w = window::make(WindowType::SqrtHann, 8);

    ASSERT_EQ(w.size(), 8);
    EXPECT_NEAR(w[0], 0.0f, 1e-6f);
    EXPECT_GT(w[4], 0.99f);
}

TEST(WindowTest, ParseKnownWindowTypes) {
    EXPECT_EQ(window::parse("rect"), WindowType::Rect);
    EXPECT_EQ(window::parse("hann"), WindowType::Hann);
    EXPECT_EQ(window::parse("sqrt-hann"), WindowType::SqrtHann);
    EXPECT_EQ(window::parse("root-hann"), WindowType::SqrtHann);
}

TEST(WindowTest, ParseUnknownWindowThrows) {
    EXPECT_THROW(window::parse("bad"), std::invalid_argument);
}