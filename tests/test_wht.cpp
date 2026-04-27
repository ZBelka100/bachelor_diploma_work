#include "wht.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <vector>

TEST(WhtTest, PowerOfTwoHelpers) {
    EXPECT_FALSE(wht::is_power_of_two(0));
    EXPECT_TRUE(wht::is_power_of_two(1));
    EXPECT_TRUE(wht::is_power_of_two(8));
    EXPECT_FALSE(wht::is_power_of_two(10));

    EXPECT_EQ(wht::next_power_of_two(0), 1);
    EXPECT_EQ(wht::next_power_of_two(1), 1);
    EXPECT_EQ(wht::next_power_of_two(9), 16);
}

TEST(WhtTest, ForwardInverseHadamardRoundTrip) {
    const std::vector<float> x = {1.0f, -2.0f, 3.0f, 4.0f, -1.0f, 0.5f, 2.5f, -3.0f};

    auto y = wht::forward(x, Ordering::Hadamard, true);
    auto z = wht::inverse(y, Ordering::Hadamard, true);

    ASSERT_EQ(z.size(), x.size());

    for (std::size_t i = 0; i < x.size(); ++i) {
        EXPECT_NEAR(z[i], x[i], 1e-5f);
    }
}

TEST(WhtTest, ForwardInverseSequencyRoundTrip) {
    const std::vector<float> x = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};

    auto y = wht::forward(x, Ordering::Sequency, true);
    auto z = wht::inverse(y, Ordering::Sequency, true);

    ASSERT_EQ(z.size(), x.size());

    for (std::size_t i = 0; i < x.size(); ++i) {
        EXPECT_NEAR(z[i], x[i], 1e-5f);
    }
}

TEST(WhtTest, RejectsNonPowerOfTwo) {
    std::vector<float> x = {1.0f, 2.0f, 3.0f};

    EXPECT_THROW(wht::fwht_inplace(x), std::invalid_argument);
    EXPECT_THROW(wht::Plan(3, Ordering::Hadamard, true), std::invalid_argument);
}