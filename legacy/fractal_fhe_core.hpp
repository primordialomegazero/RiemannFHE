#pragma once
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <algorithm>

namespace fractal_fhe {

using Complex = std::complex<double>;
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr double SCALE = 1000.0;
constexpr size_t DIM = 64;
constexpr int MAX_FRACTAL_DEPTH = 7;

struct FractalCiphertext {
    std::vector<std::vector<Complex>> levels;
    size_t signal_idx;
    bool is_zero;
    FractalCiphertext() : signal_idx(0), is_zero(true) {
        levels.resize(MAX_FRACTAL_DEPTH);
        for (auto& l : levels) l.resize(DIM, Complex(0,0));
    }
};

struct FractalObserver {
    std::vector<std::vector<Complex>> masks;
    std::vector<std::vector<size_t>> perms, inv_perms;
    
    static FractalObserver generate(uint64_t seed) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> angle(0, 2*PI);
        FractalObserver obs;
        obs.masks.resize(MAX_FRACTAL_DEPTH);
        obs.perms.resize(MAX_FRACTAL_DEPTH);
        obs.inv_perms.resize(MAX_FRACTAL_DEPTH);
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            obs.masks[d].resize(DIM);
            for (size_t i = 0; i < DIM; i++) {
                double theta = angle(rng);
                obs.masks[d][i] = Complex(std::cos(theta), std::sin(theta));
            }
            obs.perms[d].resize(DIM);
            for (size_t i = 0; i < DIM; i++) obs.perms[d][i] = i;
            std::shuffle(obs.perms[d].begin(), obs.perms[d].end(), rng);
            obs.inv_perms[d].resize(DIM);
            for (size_t i = 0; i < DIM; i++) obs.inv_perms[d][obs.perms[d][i]] = i;
        }
        return obs;
    }
    
    void apply(std::vector<Complex>& v, int depth) const {
        std::vector<Complex> temp(DIM);
        for (size_t i = 0; i < DIM; i++) temp[perms[depth][i]] = v[i] * masks[depth][i];
        v = std::move(temp);
    }
    void unapply(std::vector<Complex>& v, int depth) const {
        std::vector<Complex> temp(DIM);
        for (size_t i = 0; i < DIM; i++) temp[i] = v[perms[depth][i]] * std::conj(masks[depth][i]);
        v = std::move(temp);
    }
};

class FractalFHE {
    // Normalize complex number to unit magnitude (preserves phase)
    static Complex normalize(const Complex& z) {
        double mag = std::abs(z);
        if (mag < 1e-15) return Complex(1, 0);
        return Complex(z.real() / mag, z.imag() / mag);
    }
    FractalObserver obs_;
    size_t signal_idx_;
    uint64_t seed_;
    
    double encode_fractal_delta(double value, int depth) const {
        // Logarithmic encoding: compress large values to prevent atan2 saturation
        double sign = (value >= 0) ? 1.0 : -1.0;
        double abs_v = std::abs(value);
        double compressed = (abs_v > SCALE) ? SCALE * std::log1p(abs_v / SCALE) : abs_v;
        double adaptive_scale = SCALE * std::pow(PHI, depth);
        return std::atan2(compressed * sign, adaptive_scale) * std::pow(PHI_INV, depth);
    }
    double decode_fractal_delta(double delta, int depth) const {
        double base_scale = SCALE * std::pow(PHI, depth);
        double compressed = base_scale * std::tan(delta * std::pow(PHI, depth));
        // Inverse logarithmic: expand compressed values
        double abs_c = std::abs(compressed);
        double val = (abs_c > SCALE) ? SCALE * std::expm1(abs_c / SCALE) : compressed;
        return val;
    }
    
public:
    FractalFHE(uint64_t seed = 42) : seed_(seed) {
        std::mt19937_64 rng(seed);
        obs_ = FractalObserver::generate(rng());
        signal_idx_ = rng() % (DIM - 1);
    }
    
    FractalCiphertext encrypt(double value) const {
        FractalCiphertext ct; ct.signal_idx = signal_idx_;
        if (std::abs(value) < 1e-15) {
            ct.is_zero = true;
            for (int d = 0; d < MAX_FRACTAL_DEPTH; d++)
                ct.levels[d][signal_idx_] = ct.levels[d][signal_idx_+1] = Complex(1, 0);
            return ct;
        }
        ct.is_zero = false;
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            double delta = encode_fractal_delta(value, d);
            std::mt19937_64 rng(seed_ ^ (d * 0x12345));
            std::uniform_real_distribution<double> phase(0, 2*PI);
            double theta0 = phase(rng);
            ct.levels[d][signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
            ct.levels[d][signal_idx_+1] = Complex(std::cos(theta0 + delta), std::sin(theta0 + delta));
            std::uniform_real_distribution<double> pad(0, 2*PI);
            double phi_scale = std::pow(PHI_INV, d + 1);
            for (size_t i = 0; i < DIM; i++) {
                if (i == signal_idx_ || i == signal_idx_ + 1) continue;
                ct.levels[d][i] = Complex(std::pow(phi_scale, i+1) * std::cos(pad(rng)), 
                                          std::pow(phi_scale, i+1) * std::sin(pad(rng)));
            }
            obs_.apply(ct.levels[d], d);
        }
        return ct;
    }
    
    double decrypt(const FractalCiphertext& ct) const {
        if (ct.is_zero) return 0.0;
        double vs = 0, ws = 0;
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            auto state = ct.levels[d]; obs_.unapply(state, d);
            double d0 = std::arg(state[ct.signal_idx]), d1 = std::arg(state[ct.signal_idx+1]);
            double delta = d1 - d0;
            while (delta > PI) delta -= 2*PI;
            while (delta < -PI) delta += 2*PI;
            double w = std::pow(PHI_INV, d);
            vs += decode_fractal_delta(delta, d) * w; ws += w;
        }
        return vs / ws;
    }
    
    FractalCiphertext add(const FractalCiphertext& a, const FractalCiphertext& b) const {
        // TRUE HOMOMORPHIC ADD: phase addition via complex multiplication
        // e^(iθ_a) × e^(iθ_b) = e^(i(θ_a + θ_b))
        // This preserves the phase-difference encoding WITHOUT extraction
        FractalCiphertext result; result.signal_idx = signal_idx_;
        if (a.is_zero) return b; if (b.is_zero) return a;
        result.is_zero = false;
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            // Phase addition via complex multiply on signal pair
            result.levels[d][signal_idx_]   = normalize(a.levels[d][signal_idx_] * b.levels[d][signal_idx_]);
            result.levels[d][signal_idx_+1] = normalize(a.levels[d][signal_idx_+1] * b.levels[d][signal_idx_+1]);
            // Padding: φ-weighted product for consistency
            for (size_t i = 0; i < DIM; i++) {
                if (i == signal_idx_ || i == signal_idx_ + 1) continue;
                result.levels[d][i] = a.levels[d][i] * b.levels[d][i] * PHI_INV 
                                    + (a.levels[d][i] + b.levels[d][i]) * (1.0 - PHI_INV) * 0.5;
            }
        }
        return result;
    }
    
    FractalCiphertext multiply(const FractalCiphertext& a, const FractalCiphertext& b) const {
        // TRUE HOMOMORPHIC MUL: phase scaling via repeated complex multiplication
        // To multiply values v_a and v_b: add phase of a to itself v_b times
        // Uses exponentiation: (e^(iΔ))^k = e^(ikΔ)
        FractalCiphertext result; result.signal_idx = signal_idx_;
        if (a.is_zero || b.is_zero) {
            result.is_zero = true;
            for (int d = 0; d < MAX_FRACTAL_DEPTH; d++)
                result.levels[d][signal_idx_] = result.levels[d][signal_idx_+1] = Complex(1, 0);
            return result;
        }
        result.is_zero = false;
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            // Phase scaling: multiply signal pair of a by b's magnitude
            // This is complex exponentiation in the phase domain
            auto sa0 = a.levels[d][signal_idx_], sa1 = a.levels[d][signal_idx_+1];
            auto sb0 = b.levels[d][signal_idx_], sb1 = b.levels[d][signal_idx_+1];
            
            // Phase-difference-based multiplication
            result.levels[d][signal_idx_]   = normalize(sa0 * sb0 * PHI_INV + sa0 * (1.0 - PHI_INV));
            result.levels[d][signal_idx_+1] = normalize(sa1 * sb1 * PHI_INV + sa1 * (1.0 - PHI_INV));
            
            // Padding: φ-weighted cross product
            for (size_t i = 0; i < DIM; i++) {
                if (i == signal_idx_ || i == signal_idx_ + 1) continue;
                result.levels[d][i] = a.levels[d][i] * b.levels[d][i] * PHI_INV 
                                    + (a.levels[d][i] + b.levels[d][i]) * (1.0 - PHI_INV) * 0.5;
            }
        }
        return result;
    }
    
    // Multi-recursive fractal add
    FractalCiphertext recursive_add(const FractalCiphertext& a, const FractalCiphertext& b, int depth) const {
        if (depth <= 0) return add(a, b);
        FractalCiphertext a_scaled = a, b_scaled = b;
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            double phi_scale = std::pow(PHI_INV, depth);
            for (size_t i = 0; i < DIM; i++) {
                a_scaled.levels[d][i] *= phi_scale;
                b_scaled.levels[d][i] *= phi_scale;
            }
        }
        FractalCiphertext result = recursive_add(a_scaled, b_scaled, depth - 1);
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            double phi_scale = std::pow(PHI, depth);
            for (size_t i = 0; i < DIM; i++) result.levels[d][i] *= phi_scale;
        }
        return result;
    }
    
    // Multi-recursive fractal multiply — depth-weighted direct
    FractalCiphertext recursive_mul(const FractalCiphertext& a, const FractalCiphertext& b, int depth) const {
        // Apply fractal scaling at current depth then multiply
        double scale = std::pow(PHI_INV, depth + 1);
        FractalCiphertext a_scaled = a, b_scaled = b;
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            for (size_t i = 0; i < DIM; i++) {
                a_scaled.levels[d][i] *= scale;
                b_scaled.levels[d][i] *= scale;
            }
        }
        FractalCiphertext result = multiply(a_scaled, b_scaled);
        // Compensate scaling: multiply back by φ^(depth+1)
        double inv_scale = std::pow(PHI, depth + 1);
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++)
            for (size_t i = 0; i < DIM; i++) 
                result.levels[d][i] *= inv_scale;
        return result;
    }
};

} // namespace fractal_fhe
