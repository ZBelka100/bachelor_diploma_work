#pragma once

#include "types.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace wht {

bool is_power_of_two(std::size_t n);
std::size_t next_power_of_two(std::size_t n);

std::uint32_t gray_code(std::uint32_t x);
std::uint32_t inverse_gray_code(std::uint32_t g);
std::uint32_t bit_reverse(std::uint32_t x, int bits);

std::vector<std::size_t> permutation_from_hadamard(std::size_t n, Ordering ord);
std::vector<std::size_t> inverse_permutation(const std::vector<std::size_t>& p);

struct Plan {
    std::size_t n = 0;
    Ordering ordering = Ordering::Sequency;
    bool orthonormal = true;

    std::vector<std::size_t> perm;
    std::vector<std::size_t> inv_perm;
    std::vector<float> scratch;

    Plan() = default;
    Plan(std::size_t n_, Ordering ord_, bool ortho_);
};

void fwht_inplace(float* a, std::size_t n);
void fwht_inplace(std::vector<float>& a);

void reorder_from_hadamard(const float* in, float* out, const Plan& plan);
void reorder_to_hadamard(const float* in, float* out, const Plan& plan);

void forward_inplace(float* data, Plan& plan);
void inverse_inplace(float* data, Plan& plan);

std::vector<float> forward(
    const std::vector<float>& x,
    Ordering ordering = Ordering::Sequency,
    bool orthonormal = true
);

std::vector<float> inverse(
    const std::vector<float>& y,
    Ordering ordering = Ordering::Sequency,
    bool orthonormal = true
);

} // namespace wht