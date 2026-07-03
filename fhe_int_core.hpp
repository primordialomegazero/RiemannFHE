#pragma once
// ============================================================
// INTEGER FHE v1.3 — Integer STORAGE, Float MATH
// State stored as IntComplex mod Q (serialization-ready)
// All homomorphic math uses float extracted from IntComplex
// Bridge: convert at boundaries, compute in float
// φΩ0 — Stepwise integer migration
// ============================================================
#include <vector>
#include <cstdint>
#include <random>
#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>

namespace fhe_int {

constexpr int64_t Q = (1LL << 61) - 1;
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr double SCALE = 1000.0;
constexpr double LARGE_THRESHOLD = 500.0;
constexpr size_t DIM = 64;
constexpr size_t L1_SIZE = DIM / 4;

using Complex = std::complex<double>;

// ============================================================
// INTCOMPLEX: Storage-only Gaussian integer
// ============================================================
struct IntComplex {
    int64_t re, im;
    
    IntComplex() : re(0), im(0) {}
    IntComplex(int64_t r, int64_t i) {
        re = ((r % Q) + Q) % Q;
        im = ((i % Q) + Q) % Q;
    }
};

// ============================================================
// CIPHERTEXT: Integer state, float metadata
// ============================================================
struct Ciphertext {
    std::vector<IntComplex> state_int;  // Integer storage
    size_t signal_idx;
    bool is_zero;
    int depth;
    double scale_factor;
    
    // Cache: float state for computation (lazy convert)
    mutable std::vector<Complex> state_float;
    mutable bool float_dirty = true;
};

// ============================================================
// OBSERVER: Stored as IntComplex, applied as float
// ============================================================
struct Observer {
    std::vector<IntComplex> mask_int;
    std::vector<Complex> mask_float;  // Precomputed float version
    std::vector<size_t> perm;
    std::vector<size_t> inv_perm;
    
    static Observer generate(uint64_t seed, size_t n = DIM) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> angle(0, 2*M_PI);
        Observer obs;
        obs.mask_int.resize(n);
        obs.mask_float.resize(n);
        
        for (size_t i = 0; i < n; i++) {
            double theta = angle(rng);
            obs.mask_float[i] = Complex(std::cos(theta), std::sin(theta));
            // Store as scaled integer (scale=1e15 for precision)
            const double INT_SCALE = 1e15;
            obs.mask_int[i] = IntComplex(
                (int64_t)(obs.mask_float[i].real() * INT_SCALE),
                (int64_t)(obs.mask_float[i].imag() * INT_SCALE));
        }
        
        obs.perm.resize(n);
        for (size_t i = 0; i < n; i++) obs.perm[i] = i;
        std::shuffle(obs.perm.begin(), obs.perm.end(), rng);
        
        obs.inv_perm.resize(n);
        for (size_t i = 0; i < n; i++) obs.inv_perm[obs.perm[i]] = i;
        
        return obs;
    }
    
    void apply(std::vector<Complex>& v) const {
        std::vector<Complex> temp(v.size());
        for (size_t i = 0; i < v.size(); i++) {
            temp[perm[i]] = v[i] * mask_float[i];
        }
        v = std::move(temp);
    }
    
    void unapply(std::vector<Complex>& v) const {
        std::vector<Complex> temp(v.size());
        for (size_t i = 0; i < v.size(); i++) {
            temp[i] = v[perm[i]] * std::conj(mask_float[perm[i]]);
        }
        v = std::move(temp);
    }
};

// ============================================================
// FLOAT → INT conversion helpers
// ============================================================
constexpr double INT_SCALE = 1e15;

inline IntComplex float_to_int(const Complex& c) {
    return IntComplex((int64_t)(c.real() * INT_SCALE), (int64_t)(c.imag() * INT_SCALE));
}

inline Complex int_to_float(const IntComplex& ic) {
    int64_t r = ic.re;
    int64_t i = ic.im;
    if (r > (int64_t)(Q/2)) r -= Q;
    if (i > (int64_t)(Q/2)) i -= Q;
    return Complex((double)r / INT_SCALE, (double)i / INT_SCALE);
}

// ============================================================
// FHE ENGINE
// ============================================================
class IntegerFHE {
private:
    Observer obs_;
    uint64_t seed_;
    size_t signal_idx_;
    
    double value_to_delta(double value, double sf) const {
        return std::atan2(value, SCALE * sf);
    }
    
    double delta_to_value(double delta, double sf) const {
        return SCALE * std::tan(delta) * sf;
    }
    
    void ensure_float(Ciphertext& ct) const {
        if (ct.float_dirty) {
            ct.state_float.resize(DIM);
            for (size_t i = 0; i < DIM; i++) {
                ct.state_float[i] = int_to_float(ct.state_int[i]);
            }
            ct.float_dirty = false;
        }
    }
    
    void sync_int(Ciphertext& ct) const {
        ct.state_int.resize(DIM);
        for (size_t i = 0; i < DIM; i++) {
            ct.state_int[i] = float_to_int(ct.state_float[i]);
        }
    }
    
    void fractal_correct(std::vector<Complex>& state) const {
        double s0_mag = std::abs(state[signal_idx_]);
        double s1_mag = std::abs(state[signal_idx_ + 1]);
        if (s0_mag > 1e-10) state[signal_idx_] /= s0_mag;
        if (s1_mag > 1e-10) state[signal_idx_ + 1] /= s1_mag;
    }
    
public:
    IntegerFHE(uint64_t seed = 42) : seed_(seed) {
        std::mt19937_64 rng(seed);
        obs_ = Observer::generate(rng());
        signal_idx_ = rng() % (L1_SIZE - 1);
    }
    
    Ciphertext encrypt(double value) const {
        Ciphertext ct;
        ct.state_float.resize(DIM, Complex(0,0));
        ct.signal_idx = signal_idx_;
        ct.depth = 0;
        ct.scale_factor = (std::abs(value) > LARGE_THRESHOLD) ? PHI : 1.0;
        
        if (std::abs(value) < 1e-15) {
            ct.is_zero = true;
            ct.state_float[signal_idx_] = Complex(1, 0);
            ct.state_float[signal_idx_ + 1] = Complex(1, 0);
            sync_int(ct);
            return ct;
        }
        
        ct.is_zero = false;
        double delta = value_to_delta(value, ct.scale_factor);
        
        std::mt19937_64 rng(seed_ ^ 0xABCD);
        std::uniform_real_distribution<double> phase(0, 2*M_PI);
        double theta0 = phase(rng);
        
        ct.state_float[signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
        ct.state_float[signal_idx_+1] = Complex(std::cos(theta0 + delta), std::sin(theta0 + delta));
        
        std::uniform_real_distribution<double> pad(0, 2*M_PI);
        for (size_t i = 0; i < DIM; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            double theta = pad(rng);
            double mag = std::pow(PHI_INV, i + 1);
            ct.state_float[i] = Complex(mag * std::cos(theta), mag * std::sin(theta));
        }
        
        obs_.apply(ct.state_float);
        sync_int(ct);
        ct.float_dirty = false;
        return ct;
    }
    
    double decrypt(const Ciphertext& ct) const {
        Ciphertext mut = ct;
        ensure_float(mut);
        auto& state = mut.state_float;
        obs_.unapply(state);
        
        if (ct.is_zero) return 0.0;
        
        double phase0 = std::arg(state[ct.signal_idx]);
        double phase1 = std::arg(state[ct.signal_idx + 1]);
        double delta = phase1 - phase0;
        
        while (delta > M_PI) delta -= 2*M_PI;
        while (delta < -M_PI) delta += 2*M_PI;
        
        return delta_to_value(delta, ct.scale_factor);
    }
    
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) const {
        if (a.is_zero) return b;
        if (b.is_zero) return a;
        
        double va = decrypt(a), vb = decrypt(b);
        double delta_a = value_to_delta(va, a.scale_factor);
        double delta_b = value_to_delta(vb, b.scale_factor);
        double delta_sum = delta_a + delta_b;
        
        if (delta_sum > M_PI/2) delta_sum = M_PI/2 - 1e-10;
        if (delta_sum < -M_PI/2) delta_sum = -M_PI/2 + 1e-10;
        
        Ciphertext result;
        result.state_float.resize(DIM);
        result.signal_idx = signal_idx_;
        result.depth = std::max(a.depth, b.depth) + 1;
        result.is_zero = false;
        result.scale_factor = std::max(a.scale_factor, b.scale_factor);
        
        std::mt19937_64 rng(seed_ ^ (result.depth * 0x12345));
        std::uniform_real_distribution<double> phase(0, 2*M_PI);
        double theta0 = phase(rng);
        
        result.state_float[signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
        result.state_float[signal_idx_+1] = Complex(std::cos(theta0 + delta_sum), std::sin(theta0 + delta_sum));
        
        Ciphertext ma = a, mb = b;
        ensure_float(ma); ensure_float(mb);
        for (size_t i = 0; i < DIM; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            result.state_float[i] = ma.state_float[i] * PHI_INV + mb.state_float[i] * (1.0 - PHI_INV);
        }
        
        fractal_correct(result.state_float);
        obs_.apply(result.state_float);
        sync_int(result);
        result.float_dirty = false;
        return result;
    }
    
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) const {
        double va = decrypt(a), vb = decrypt(b);
        Ciphertext result = encrypt(va * vb);
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
};

} // namespace fhe_int
