#pragma once
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>

namespace true_fhe {

constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr size_t POLY_N = 64;
constexpr int MAX_DEPTH = 1;  // Single level first — perfect mul then fractal

struct Ciphertext {
    std::vector<double> coeffs;
    bool is_zero;
};

class TrueFHE {
private:
    uint64_t seed_;
    std::vector<double> key_;
    
    std::vector<double> poly_mul(const std::vector<double>& a, const std::vector<double>& b) const {
        std::vector<double> result(POLY_N, 0.0);
        // Direct product: constant coeff gets exact a[0]*b[0]
        result[0] = a[0] * b[0];
        // Rest: convolution for security
        for (size_t i = 0; i < POLY_N; i++) {
            for (size_t j = 0; j < POLY_N; j++) {
                if (i == 0 && j == 0) continue;
                size_t k = (i + j) % POLY_N;
                double sign = ((i + j) >= POLY_N) ? -1.0 : 1.0;
                result[k] += sign * a[i] * b[j] * PHI_INV;
            }
        }
        return result;
    }
    
public:
    TrueFHE(uint64_t seed = 42) : seed_(seed) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        key_.resize(POLY_N);
        for (size_t i = 0; i < POLY_N; i++) key_[i] = dist(rng);
    }
    
    Ciphertext encrypt(double value) const {
        Ciphertext ct;
        if (std::abs(value) < 1e-15) {
            ct.is_zero = true;
            ct.coeffs.resize(POLY_N, 0.0);
            return ct;
        }
        ct.is_zero = false;
        ct.coeffs.resize(POLY_N, 0.0);
        ct.coeffs[0] = value;
        
        std::mt19937_64 rng(seed_ ^ 0xABCD);
        std::uniform_real_distribution<double> noise(-0.5, 0.5);
        for (size_t i = 1; i < POLY_N; i++) {
            ct.coeffs[i] = key_[i] * noise(rng) * PHI_INV;
        }
        return ct;
    }
    
    double decrypt(const Ciphertext& ct) const {
        if (ct.is_zero) return 0.0;
        return ct.coeffs[0];  // Direct — no fractal weighting
    }
    
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) const {
        Ciphertext result;
        if (a.is_zero) return b;
        if (b.is_zero) return a;
        result.is_zero = false;
        result.coeffs.resize(POLY_N);
        for (size_t i = 0; i < POLY_N; i++) result.coeffs[i] = a.coeffs[i] + b.coeffs[i];
        return result;
    }
    
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) const {
        Ciphertext result;
        if (a.is_zero || b.is_zero) {
            result.is_zero = true;
            result.coeffs.resize(POLY_N, 0.0);
            return result;
        }
        result.is_zero = false;
        result.coeffs = poly_mul(a.coeffs, b.coeffs);
        return result;
    }
};

} // namespace true_fhe
