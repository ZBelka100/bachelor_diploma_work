#include "wht.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace wht {

bool is_power_of_two(std::size_t n) {
    return n != 0 && ((n & (n - 1)) == 0);
}

std::size_t next_power_of_two(std::size_t n) {
    if (n == 0) return 1;
    std::size_t p = 1;
    while (p < n) p <<= 1U;
    return p;
}

std::uint32_t gray_code(std::uint32_t x) {
    return x ^ (x >> 1U);
}

std::uint32_t inverse_gray_code(std::uint32_t g) {
    std::uint32_t x = 0;
    for (; g; g >>= 1U) x ^= g;
    return x;
}

std::uint32_t bit_reverse(std::uint32_t x, int bits) {
    std::uint32_t r = 0;
    for (int i = 0; i < bits; ++i) {
        r = (r << 1U) | (x & 1U);
        x >>= 1U;
    }
    return r;
}

std::vector<std::size_t> permutation_from_hadamard(std::size_t n, Ordering ord) {
    if (!is_power_of_two(n)) {
        throw std::invalid_argument("permutation_from_hadamard: n must be power of two");
    }

    int bits = 0;
    for (std::size_t t = n; t > 1; t >>= 1U) ++bits;

    std::vector<std::size_t> p(n);

    if (ord == Ordering::Hadamard) {
        for (std::size_t i = 0; i < n; ++i) p[i] = i;
        return p;
    }

    if (ord == Ordering::Sequency) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto g = gray_code(static_cast<std::uint32_t>(i));
            p[i] = static_cast<std::size_t>(bit_reverse(g, bits));
        }
        return p;
    }

    for (std::size_t i = 0; i < n; ++i) {
        const auto s =
            static_cast<std::uint32_t>(bit_reverse(gray_code(static_cast<std::uint32_t>(i)), bits));
        p[i] = static_cast<std::size_t>(inverse_gray_code(s));
    }
    return p;
}

std::vector<std::size_t> inverse_permutation(const std::vector<std::size_t>& p) {
    std::vector<std::size_t> inv(p.size());
    for (std::size_t i = 0; i < p.size(); ++i) {
        inv[p[i]] = i;
    }
    return inv;
}

Plan::Plan(std::size_t n_, Ordering ord_, bool ortho_)
    : n(n_), ordering(ord_), orthonormal(ortho_) {
    if (!is_power_of_two(n)) {
        throw std::invalid_argument("wht::Plan: n must be power of two");
    }
    perm = permutation_from_hadamard(n, ordering);
    inv_perm = inverse_permutation(perm);
    scratch.resize(n, 0.0f);
}

void fwht_inplace(float* a, std::size_t n) {
    if (!is_power_of_two(n)) {
        throw std::invalid_argument("fwht_inplace: n must be power of two");
    }

    for (std::size_t h = 1; h < n; h <<= 1U) {
        const std::size_t step = h << 1U;
        for (std::size_t i = 0; i < n; i += step) {
            float* left = a + i;
            float* right = a + i + h;
            for (std::size_t j = 0; j < h; ++j) {
                const float x = left[j];
                const float y = right[j];
                left[j] = x + y;
                right[j] = x - y;
            }
        }
    }
}

void fwht_inplace(std::vector<float>& a) {
    fwht_inplace(a.data(), a.size());
}

void reorder_from_hadamard(const float* in, float* out, const Plan& plan) {
    for (std::size_t i = 0; i < plan.n; ++i) {
        out[plan.perm[i]] = in[i];
    }
}

void reorder_to_hadamard(const float* in, float* out, const Plan& plan) {
    for (std::size_t i = 0; i < plan.n; ++i) {
        out[plan.inv_perm[i]] = in[i];
    }
}

void forward_inplace(float* data, Plan& plan) {
    fwht_inplace(data, plan.n);

    if (plan.orthonormal) {
        const float s = 1.0f / std::sqrt(static_cast<float>(plan.n));
        for (std::size_t i = 0; i < plan.n; ++i) data[i] *= s;
    }

    if (plan.ordering != Ordering::Hadamard) {
        reorder_from_hadamard(data, plan.scratch.data(), plan);
        std::memcpy(data, plan.scratch.data(), plan.n * sizeof(float));
    }
}

void inverse_inplace(float* data, Plan& plan) {
    if (plan.ordering != Ordering::Hadamard) {
        reorder_to_hadamard(data, plan.scratch.data(), plan);
        std::memcpy(data, plan.scratch.data(), plan.n * sizeof(float));
    }

    fwht_inplace(data, plan.n);

    if (plan.orthonormal) {
        const float s = 1.0f / std::sqrt(static_cast<float>(plan.n));
        for (std::size_t i = 0; i < plan.n; ++i) data[i] *= s;
    } else {
        const float s = 1.0f / static_cast<float>(plan.n);
        for (std::size_t i = 0; i < plan.n; ++i) data[i] *= s;
    }
}

std::vector<float> forward(const std::vector<float>& x, Ordering ordering, bool orthonormal) {
    Plan plan(x.size(), ordering, orthonormal);
    std::vector<float> y = x;
    forward_inplace(y.data(), plan);
    return y;
}

std::vector<float> inverse(const std::vector<float>& y, Ordering ordering, bool orthonormal) {
    Plan plan(y.size(), ordering, orthonormal);
    std::vector<float> x = y;
    inverse_inplace(x.data(), plan);
    return x;
}

} // namespace wht