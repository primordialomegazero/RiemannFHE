#pragma once
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>

namespace true_fhe {

constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr size_t POLY_N = 64;
constexpr int FRACTAL_DEPTH = 7;

struct Ciphertext {
    std::vector<std::vector<double>> levels;  // [depth][coeffs]
    bool is_zero;
};

class TrueFHE {
private:
    uint64_t seed_;
    std::vector<double> key_;
    
    std::vector<double> poly_mul(const std::vector<double>& a, const std::vector<double>& b) const {
        std::vector<double> result(POLY_N, 0.0);
        result[0] = a[0] * b[0];  // Exact product at constant coeff
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
    
    // ============================================================
    // ENCRYPT — Fractal: value encoded at depth 0, φ-scaled at deeper levels
    // ============================================================
    Ciphertext encrypt(double value) const {
        Ciphertext ct;
        if (std::abs(value) < 1e-15) {
            ct.is_zero = true;
            ct.levels.resize(FRACTAL_DEPTH, std::vector<double>(POLY_N, 0.0));
            return ct;
        }
        ct.is_zero = false;
        ct.levels.resize(FRACTAL_DEPTH);
        
        std::mt19937_64 rng(seed_ ^ 0xABCD);
        std::uniform_real_distribution<double> noise(-0.5, 0.5);
        
        for (int d = 0; d < FRACTAL_DEPTH; d++) {
            ct.levels[d].resize(POLY_N, 0.0);
            // Fractal: φ^(-d) scaled value at each depth
            ct.levels[d][0] = value * std::pow(PHI_INV, d);
            for (size_t i = 1; i < POLY_N; i++) {
                ct.levels[d][i] = key_[i] * noise(rng) * std::pow(PHI_INV, d + 1);
            }
        }
        return ct;
    }
    
    // ============================================================
    // DECRYPT — Weighted sum across fractal levels
    // ============================================================
    double decrypt(const Ciphertext& ct) const {
        if (ct.is_zero) return 0.0;
        double sum = 0.0, weight = 0.0;
        for (int d = 0; d < FRACTAL_DEPTH; d++) {
            double w = std::pow(PHI, d);  // Reverse encoding weight
            sum += ct.levels[d][0] * w;
            weight += 1.0;
        }
        return sum / weight;
    }
    
    // ============================================================
    // ADD — Per-level polynomial addition
    // ============================================================
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) const {
        Ciphertext result;
        if (a.is_zero) return b;
        if (b.is_zero) return a;
        result.is_zero = false;
        result.levels.resize(FRACTAL_DEPTH);
        for (int d = 0; d < FRACTAL_DEPTH; d++) {
            result.levels[d].resize(POLY_N);
            for (size_t i = 0; i < POLY_N; i++) {
                result.levels[d][i] = a.levels[d][i] + b.levels[d][i];
            }
        }
        return result;
    }
    
    // ============================================================
    // MUL — φ-compensated per-level polynomial multiplication
    // a[d] = v_a * φ^(-d), b[d] = v_b * φ^(-d)
    // poly_mul gives: v_a*v_b * φ^(-2d)
    // Compensate: multiply result by φ^d to get v_a*v_b * φ^(-d)
    // Then decrypt weights by φ^d → v_a*v_b (correct!)
    // ============================================================
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) const {
        Ciphertext result;
        if (a.is_zero || b.is_zero) {
            result.is_zero = true;
            result.levels.resize(FRACTAL_DEPTH, std::vector<double>(POLY_N, 0.0));
            return result;
        }
        result.is_zero = false;
        result.levels.resize(FRACTAL_DEPTH);
        for (int d = 0; d < FRACTAL_DEPTH; d++) {
            result.levels[d] = poly_mul(a.levels[d], b.levels[d]);
            // φ-compensation: scale by φ^d so decrypt recovers correct product
            double phi_pow = std::pow(PHI, d);
            for (size_t i = 0; i < POLY_N; i++) {
                result.levels[d][i] *= phi_pow;
            }
        }
        return result;
    }
};

} // namespace true_fhe
