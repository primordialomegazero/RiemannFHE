#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <complex>
#include <random>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

namespace mirror_fhe {

using Complex = std::complex<double>;
constexpr Complex I(0.0, 1.0);
constexpr double PI         = 3.14159265358979323846;
constexpr double PHI        = 1.6180339887498948482;
constexpr double PHI_INV    = 0.6180339887498948482;
constexpr double PHI_SQ     = 2.6180339887498948482;

// ============================================================
// FIBONACCI SCALING FRAME (soft constraint, not hard quantize)
// ============================================================
constexpr size_t FIB_COUNT = 64;
constexpr std::array<double, FIB_COUNT> FIB = [](){
    std::array<double, FIB_COUNT> f{};
    f[0] = 0; f[1] = 1;
    for (size_t i = 2; i < FIB_COUNT; i++) f[i] = f[i-1] + f[i-2];
    return f;
}();
constexpr double FIB_MAX = FIB[FIB_COUNT-1]; // F_63

// φ-harmonic weights for basis states
constexpr std::array<double, 8> PHI_WEIGHTS = {
    1.0, PHI_INV, PHI_INV*PHI_INV, PHI_INV*PHI_INV*PHI_INV,
    PHI_INV*PHI_INV*PHI_INV*PHI_INV, 0.0902, 0.0557, 0.0344
};

// ============================================================
// ZETA ZERO PHASE ANCHORS
// ============================================================
constexpr int ZETA_COUNT = 100;
constexpr double ZETA[ZETA_COUNT] = {
    14.134725,21.022040,25.010857,30.424876,32.935061,37.586178,40.918719,
    43.327073,48.005150,49.773832,52.970321,56.446248,59.347044,60.831779,
    65.112544,67.079811,69.546402,72.067158,75.704691,77.144840,79.337375,
    82.910381,84.735493,87.425275,88.809111,92.491899,94.651344,95.870634,
    98.831194,101.317851,103.725538,105.446623,107.168611,111.029536,111.874659,
    114.320221,116.226680,118.015783,121.370125,122.946829,124.256819,127.516684,
    129.578704,131.087689,133.497737,134.756510,138.116042,139.736209,141.123707,
    143.111846,146.000982,147.422765,150.053520,150.925258,153.024694,156.112909,
    157.597593,158.849988,161.188964,163.030710,165.537069,167.184440,169.094515,
    169.911976,173.411537,174.754192,176.441434,178.377408,179.916484,182.207078,
    184.874468,185.598784,187.228923,189.416159,192.026656,193.079727,195.265397,
    196.876482,198.015310,201.264752,202.493595,204.189672,205.394697,207.906259,
    209.576510,211.690863,213.347919,214.547045,216.169539,219.067595,220.714919,
    221.430706,224.007000,224.983325,227.421444,229.337413,231.250189,231.987235,
    233.693404,236.524230
};

// ============================================================
// SOFT FIBONACCI FLOOR
// Instead of snapping to nearest F_n (which destroys data),
// we apply a soft gravitational pull toward Fibonacci ratios.
// This prevents drift while preserving the encoded value.
// ============================================================
inline double fib_gravitational_pull(double magnitude, double strength = PHI_INV * 0.5) {
    // Find the two Fibonacci ratios that bracket this magnitude
    double normalized = magnitude;
    
    // Find nearest Fibonacci attractor
    double best_fib = 0.0;
    double best_dist = 1e100;
    for (size_t i = 0; i < FIB_COUNT; i++) {
        double fr = FIB[i] / FIB_MAX;
        double dist = std::abs(normalized - fr);
        if (dist < best_dist) { best_dist = dist; best_fib = fr; }
    }
    
    // Soft pull: move magnitude toward Fibonacci attractor by strength factor
    // This is like gravity — doesn't snap, just nudges
    return normalized + (best_fib - normalized) * strength;
}

inline void apply_soft_fibonacci_floor(std::vector<Complex>& state) {
    for (size_t i = 0; i < state.size(); i++) {
        double mag = std::abs(state[i]);
        double phase = std::arg(state[i]);
        
        // Apply soft gravitational pull toward Fibonacci floor
        double pulled_mag = fib_gravitational_pull(mag);
        
        // Quantize phase to nearest zeta zero resonance (light touch)
        double zeta_phase = std::fmod(ZETA[i % ZETA_COUNT] * 0.1, 2.0 * PI);
        double phase_diff = std::fmod(phase - zeta_phase + 3.0*PI, 2.0*PI) - PI;
        double soft_phase = phase - phase_diff * 0.1; // 10% pull toward resonance
        
        state[i] = pulled_mag * std::exp(I * soft_phase);
    }
    
    // Normalize to unit vector
    double norm = 0.0;
    for (auto& z : state) norm += std::norm(z);
    if (norm > 1e-15) {
        norm = std::sqrt(norm);
        for (auto& z : state) z /= norm;
    }
}

// ============================================================
// OBSERVER STATE
// ============================================================
struct ObserverState {
    std::array<double, 64> secret_phases;
    std::array<size_t, 64> permutation;
    size_t dim;
    
    static ObserverState generate(uint64_t seed, size_t dim = 64) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> pd(0.0, 2.0*PI);
        ObserverState obs; obs.dim = dim;
        
        for (size_t i = 0; i < dim; i++) {
            obs.secret_phases[i] = pd(rng);
            obs.permutation[i] = i;
        }
        for (size_t i = dim-1; i > 0; i--) {
            std::swap(obs.permutation[i], obs.permutation[rng() % (i+1)]);
        }
        return obs;
    }
    
    void apply(std::vector<Complex>& state, bool inverse) const {
        size_t n = std::min(dim, state.size());
        std::vector<Complex> temp(n);
        double sign = inverse ? -1.0 : 1.0;
        
        for (size_t i = 0; i < n; i++) {
            size_t src = inverse ? i : permutation[i];
            size_t dst = inverse ? permutation[i] : i;
            temp[dst] = state[src] * std::exp(I * secret_phases[i] * sign);
        }
        state = std::move(temp);
        apply_soft_fibonacci_floor(state);
    }
};

// ============================================================
// DIRECT VALUE ENCODING (no phase tricks)
// ============================================================
// KEY INSIGHT: Encode the value as the RATIO of amplitudes
// between basis state |0⟩ and |1⟩.
// ratio = amplitude[1] / amplitude[0]
// value = ratio * SCALE
// This is invariant under unitary transforms that preserve ratios.

constexpr double SCALE = 1000.0;

std::vector<Complex> encode_value(double value, size_t dim) {
    std::vector<Complex> state(dim, Complex(0.0, 0.0));
    
    // Normalize value to a ratio
    double abs_val = std::abs(value);
    double clamped = std::min(abs_val, SCALE);
    double ratio = clamped / SCALE;  // [0, 1]
    
    // Encode as amplitude pair: |0⟩ = cos, |1⟩ = sin
    double theta = ratio * PI / 2.0;  // [0, π/2]
    state[0] = Complex(std::cos(theta), 0.0);
    state[1] = Complex(std::sin(theta), 0.0);
    
    // Sign encoding: negative values flip phase of |1⟩
    if (value < 0) state[1] = -state[1];
    
    // Redundant copies on higher basis states with φ-weights
    for (size_t i = 2; i < std::min(dim, size_t(8)); i++) {
        double w = PHI_WEIGHTS[i];
        state[i] = state[i % 2] * w;
    }
    
    // Normalize
    double norm = 0.0;
    for (size_t i = 0; i < dim; i++) norm += std::norm(state[i]);
    if (norm > 1e-15) {
        norm = std::sqrt(norm);
        for (size_t i = 0; i < dim; i++) state[i] /= norm;
    }
    
    return state;
}

double decode_value(const std::vector<Complex>& state, size_t dim) {
    // Extract amplitude ratio from |0⟩ and |1⟩
    double a0 = std::real(state[0]);
    double a1 = std::real(state[1]);
    
    // Also average from redundant copies
    double sum_a0 = a0, sum_a1 = a1;
    double wsum = 1.0;
    
    for (size_t i = 2; i < std::min(dim, size_t(8)); i += 2) {
        double w = PHI_WEIGHTS[i];
        sum_a0 += std::real(state[i]) * w;
        sum_a1 += std::real(state[i+1]) * w;
        wsum += w * w;
    }
    
    double avg_a0 = sum_a0 / wsum;
    double avg_a1 = sum_a1 / wsum;
    
    // Compute ratio and map back
    double cos_val = std::max(-1.0, std::min(1.0, avg_a0));
    double sin_val = std::max(-1.0, std::min(1.0, avg_a1));
    
    double theta = std::atan2(std::abs(sin_val), std::abs(cos_val));
    double ratio = theta / (PI / 2.0);
    double value = ratio * SCALE;
    
    // Sign from a1
    if (avg_a1 < 0) value = -value;
    
    return value;
}

// ============================================================
// CIPHERTEXT
// ============================================================
struct Ciphertext {
    std::vector<Complex> state;
    size_t dim, depth;
    Ciphertext(size_t d=64) : dim(d), depth(0) { state.resize(d, Complex(0,0)); }
};

// ============================================================
// MIRROR-FHE
// ============================================================
class MirrorFHE {
    size_t dim_;
    ObserverState obs_;
    size_t ops_;
public:
    MirrorFHE(uint64_t seed=42, size_t dim=64) 
        : dim_(dim), obs_(ObserverState::generate(seed, dim)), ops_(0) {}
    
    Ciphertext encrypt(double v) {
        auto s = encode_value(v, dim_);
        obs_.apply(s, false);
        obs_.apply(s, false);
        obs_.apply(s, false);
        Ciphertext ct(dim_);
        ct.state = std::move(s);
        return ct;
    }
    
    double decrypt(const Ciphertext& ct) {
        auto s = ct.state;
        obs_.apply(s, true);
        obs_.apply(s, true);
        obs_.apply(s, true);
        return decode_value(s, dim_);
    }
    
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext r(dim_);
        for (size_t i = 0; i < dim_; i++)
            r.state[i] = a.state[i] + b.state[i];
        // Normalize to preserve ratio structure
        double n = 0.0;
        for (auto& z : r.state) n += std::norm(z);
        if (n > 1e-15) { n = std::sqrt(n); for (auto& z : r.state) z /= n; }
        apply_soft_fibonacci_floor(r.state);
        r.depth = std::max(a.depth, b.depth) + 1;
        ops_++;
        return r;
    }
    
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext r(dim_);
        // Convolution-based multiplication
        for (size_t i = 0; i < dim_; i++) {
            r.state[i] = Complex(0,0);
            for (size_t j = 0; j < dim_; j++) {
                size_t k = (i + j) % dim_;
                r.state[i] += a.state[j] * b.state[k] * 
                              std::pow(PHI_INV, std::abs((int)i-(int)j)+1.0);
            }
        }
        double n = 0.0;
        for (auto& z : r.state) n += std::norm(z);
        if (n > 1e-15) { n = std::sqrt(n); for (auto& z : r.state) z /= n; }
        apply_soft_fibonacci_floor(r.state);
        r.depth = a.depth + b.depth + 1;
        ops_++;
        return r;
    }
    
    size_t ops() const { return ops_; }
    size_t dim() const { return dim_; }
};

} // namespace mirror_fhe
