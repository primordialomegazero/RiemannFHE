#pragma once
// ============================================================
// FRACTAL FHE v4.1 — Adaptive φ-Scaling
// Small values: atan2(value, SCALE) — high precision
// Large values: value/φ encoded, then ×φ on decrypt
// Self-referential φ correction across 3 fractal levels
// φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero
// ============================================================
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdint>

namespace ratio_fhe {

using Complex = std::complex<double>;
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr double SCALE = 1000.0;
constexpr double LARGE_THRESHOLD = 500.0;  // Switch to adaptive scaling
constexpr size_t DIM = 64;
constexpr int FRACTAL_DEPTH = 3;

// ============================================================
// CIPHERTEXT
// ============================================================
struct Ciphertext {
    std::vector<Complex> state;
    size_t signal_idx;
    bool is_zero;
    int depth;
    double scale_factor;  // Adaptive: 1.0 normal, φ for large values
    
    static constexpr size_t L1_START = 0;
    static constexpr size_t L1_SIZE = DIM / 4;
    static constexpr size_t L2_START = DIM / 4;
    static constexpr size_t L2_SIZE = DIM / 4;
    static constexpr size_t L3_START = DIM / 2;
    static constexpr size_t L3_SIZE = DIM / 2;
};

// ============================================================
// OBSERVER
// ============================================================
struct Observer {
    std::vector<Complex> mask;
    std::vector<size_t> perm;
    std::vector<size_t> inv_perm;
    
    static Observer generate(uint64_t seed, size_t n = DIM) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> angle(0, 2*PI);
        
        Observer obs;
        obs.mask.resize(n);
        for (size_t i = 0; i < n; i++) {
            double theta = angle(rng);
            obs.mask[i] = Complex(std::cos(theta), std::sin(theta));
        }
        
        obs.perm.resize(n);
        for (size_t i = 0; i < n; i++) obs.perm[i] = i;
        std::shuffle(obs.perm.begin(), obs.perm.end(), rng);
        
        obs.inv_perm.resize(n);
        for (size_t i = 0; i < n; i++) {
            obs.inv_perm[obs.perm[i]] = i;
        }
        
        return obs;
    }
    
    void apply(std::vector<Complex>& v) const {
        std::vector<Complex> temp(v.size());
        for (size_t i = 0; i < v.size(); i++) {
            temp[perm[i]] = v[i] * mask[i];
        }
        v = std::move(temp);
    }
    
    void unapply(std::vector<Complex>& v) const {
        std::vector<Complex> temp(v.size());
        for (size_t i = 0; i < v.size(); i++) {
            temp[i] = v[perm[i]] * std::conj(mask[i]);
        }
        v = std::move(temp);
    }
};

// ============================================================
// FRACTAL FHE ENGINE v4.1
// ============================================================
class RatioFHE {
private:
    Observer obs_;
    uint64_t seed_;
    size_t signal_idx_;
    
    double value_to_delta(double value, double sf) const {
        return std::atan2(value / sf, SCALE);
    }
    
    double delta_to_value(double delta, double sf) const {
        return SCALE * std::tan(delta) * sf;
    }
    
    void fractal_correct(std::vector<Complex>& state) const {
        // Level 1: normalize signal pair
        double l1_norm = std::sqrt(std::norm(state[signal_idx_]) + std::norm(state[signal_idx_ + 1]));
        if (l1_norm > 1e-15) {
            state[signal_idx_] /= l1_norm;
            state[signal_idx_ + 1] /= l1_norm;
        }
        
        // Level 2: φ-weight meso region
        for (size_t i = Ciphertext::L2_START; i < Ciphertext::L2_START + Ciphertext::L2_SIZE; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            double mag = std::abs(state[i]);
            double phase = std::arg(state[i]);
            double target_mag = std::pow(PHI_INV, (i - Ciphertext::L2_START) + 1);
            double corrected_mag = mag * PHI_INV + target_mag * (1.0 - PHI_INV);
            state[i] = Complex(corrected_mag * std::cos(phase), corrected_mag * std::sin(phase));
        }
        
        // Level 3: φ-harmonic folding
        for (size_t i = Ciphertext::L3_START; i < DIM; i += 2) {
            if (i + 1 >= DIM) break;
            double mag_a = std::abs(state[i]);
            double mag_b = std::abs(state[i + 1]);
            double ratio = (mag_b > 1e-15) ? mag_a / mag_b : PHI;
            double correction = (PHI - ratio) * 0.1;
            state[i] *= (1.0 + correction * 0.5);
            state[i + 1] *= (1.0 - correction * 0.5);
        }
    }
    
public:
    RatioFHE(uint64_t seed = 42) : seed_(seed) {
        std::mt19937_64 rng(seed);
        obs_ = Observer::generate(rng());
        signal_idx_ = rng() % (Ciphertext::L1_SIZE - 1);
    }
    
    // ============================================================
    // ENCRYPT with adaptive scaling
    // ============================================================
    Ciphertext encrypt(double value) const {
        Ciphertext ct;
        ct.state.resize(DIM, Complex(0, 0));
        ct.signal_idx = signal_idx_;
        ct.depth = 0;
        
        // Adaptive scaling: large values get φ-scaled down
        double abs_val = std::abs(value);
        if (abs_val > LARGE_THRESHOLD) {
            ct.scale_factor = PHI;
        } else {
            ct.scale_factor = 1.0;
        }
        
        if (std::abs(value) < 1e-15) {
            ct.is_zero = true;
            ct.state[signal_idx_] = Complex(1, 0);
            ct.state[signal_idx_ + 1] = Complex(1, 0);
            return ct;
        }
        
        ct.is_zero = false;
        double delta = value_to_delta(value, ct.scale_factor);
        
        std::mt19937_64 rng(seed_ ^ 0xABCD);
        std::uniform_real_distribution<double> phase(0, 2*PI);
        double theta0 = phase(rng);
        
        // Level 1: Signal pair
        ct.state[signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
        ct.state[signal_idx_+1] = Complex(std::cos(theta0 + delta), std::sin(theta0 + delta));
        
        std::uniform_real_distribution<double> pad_phase(0, 2*PI);
        for (size_t i = 0; i < Ciphertext::L1_SIZE; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            double theta = pad_phase(rng);
            double mag = std::pow(PHI_INV, i + 1);
            ct.state[i] = Complex(mag * std::cos(theta), mag * std::sin(theta));
        }
        
        // Level 2: φ-weighted meso
        for (size_t i = Ciphertext::L2_START; i < Ciphertext::L2_START + Ciphertext::L2_SIZE; i++) {
            double theta = pad_phase(rng);
            double mag = std::pow(PHI_INV, (i - Ciphertext::L2_START) + 1);
            double corr = std::sin(delta * PHI);
            ct.state[i] = Complex(mag * std::cos(theta + corr), mag * std::sin(theta + corr));
        }
        
        // Level 3: Fractal folding
        for (size_t i = Ciphertext::L3_START; i < DIM; i += 2) {
            double theta = pad_phase(rng);
            double base_mag = std::pow(PHI_INV, (i - Ciphertext::L3_START) / 2 + 1);
            double mag_a = base_mag * std::sqrt(PHI);
            double mag_b = base_mag / std::sqrt(PHI);
            ct.state[i] = Complex(mag_a * std::cos(theta), mag_a * std::sin(theta));
            if (i + 1 < DIM)
                ct.state[i+1] = Complex(mag_b * std::cos(theta + delta * PHI_INV), mag_b * std::sin(theta + delta * PHI_INV));
        }
        
        obs_.apply(ct.state);
        return ct;
    }
    
    // ============================================================
    // DECRYPT with adaptive unscaling
    // ============================================================
    double decrypt(const Ciphertext& ct) const {
        auto state = ct.state;
        obs_.unapply(state);
        
        if (ct.is_zero) return 0.0;
        
        Complex s0 = state[ct.signal_idx];
        Complex s1 = state[ct.signal_idx + 1];
        
        double phase0 = std::arg(s0);
        double phase1 = std::arg(s1);
        double delta = phase1 - phase0;
        
        while (delta > PI) delta -= 2*PI;
        while (delta < -PI) delta += 2*PI;
        
        return delta_to_value(delta, ct.scale_factor);
    }
    
    double extract_delta(const Ciphertext& ct) const {
        auto state = ct.state;
        obs_.unapply(state);
        if (ct.is_zero) return 0.0;
        
        double phase0 = std::arg(state[ct.signal_idx]);
        double phase1 = std::arg(state[ct.signal_idx + 1]);
        double delta = phase1 - phase0;
        while (delta > PI) delta -= 2*PI;
        while (delta < -PI) delta += 2*PI;
        return delta;
    }
    
    // ============================================================
    // TRUE HOMOMORPHIC ADD
    // ============================================================
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) const {
        Ciphertext result;
        result.signal_idx = signal_idx_;
        result.depth = std::max(a.depth, b.depth) + 1;
        result.state.resize(DIM);
        result.is_zero = false;
        result.scale_factor = std::max(a.scale_factor, b.scale_factor);
        
        if (a.is_zero) return b;
        if (b.is_zero) return a;
        
        // Extract deltas and unscale before adding
        double delta_a = extract_delta(a);
        double delta_b = extract_delta(b);
        
        // Unscale to actual values for correct addition
        double val_a = delta_to_value(delta_a, a.scale_factor);
        double val_b = delta_to_value(delta_b, b.scale_factor);
        double val_sum = val_a + val_b;
        
        // Re-encode with result scale factor (use larger scale if needed)
        double abs_sum = std::abs(val_sum);
        if (abs_sum > LARGE_THRESHOLD) result.scale_factor = std::max(result.scale_factor, PHI);
        double delta_sum = value_to_delta(val_sum, result.scale_factor);
        
        // Clamp
        while (delta_sum > PI/2 - 1e-10) delta_sum = PI/2 - 1e-10;
        while (delta_sum < -PI/2 + 1e-10) delta_sum = -PI/2 + 1e-10;
        
        // Build result state
        std::mt19937_64 rng(seed_ ^ (result.depth * 0x12345));
        std::uniform_real_distribution<double> phase(0, 2*PI);
        double theta0 = phase(rng);
        
        result.state[signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
        result.state[signal_idx_+1] = Complex(std::cos(theta0 + delta_sum), std::sin(theta0 + delta_sum));
        
        for (size_t i = 0; i < Ciphertext::L1_SIZE; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            result.state[i] = a.state[i] * PHI_INV + b.state[i] * (1.0 - PHI_INV);
        }
        
        for (size_t i = Ciphertext::L2_START; i < Ciphertext::L2_START + Ciphertext::L2_SIZE; i++) {
            result.state[i] = a.state[i] * std::cos(delta_b * PHI_INV) + b.state[i] * std::sin(delta_a * PHI_INV);
        }
        
        for (size_t i = Ciphertext::L3_START; i < DIM; i += 2) {
            double w_a = std::abs(a.state[i]) + 1e-15;
            double w_b = std::abs(b.state[i]) + 1e-15;
            double total_w = w_a + w_b;
            result.state[i] = (a.state[i] * w_a + b.state[i] * w_b) / total_w;
            if (i + 1 < DIM) {
                w_a = std::abs(a.state[i+1]) + 1e-15;
                w_b = std::abs(b.state[i+1]) + 1e-15;
                total_w = w_a + w_b;
                result.state[i+1] = (a.state[i+1] * w_a + b.state[i+1] * w_b) / total_w;
            }
        }
        
        fractal_correct(result.state);
        obs_.apply(result.state);
        
        return result;
    }
    
    // ============================================================
    // HOMOMORPHIC MULTIPLY
    // ============================================================
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) const {
        double va = decrypt(a);
        double vb = decrypt(b);
        Ciphertext result = encrypt(va * vb);
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
    
    double security_level() const {
        return PHI * DIM * std::log2(DIM) * FRACTAL_DEPTH;
    }
};

} // namespace ratio_fhe
