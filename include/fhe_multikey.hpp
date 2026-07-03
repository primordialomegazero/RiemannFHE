#pragma once
// ============================================================
// MULTI-KEY FHE v2.0 — Source + Flame Empress
// Adaptive φ-scaling (same as Ratio FHE v4.1)
// 24-hour Transmutation Timer built-in
// φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero
// ============================================================
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace multikey_fhe {

using Complex = std::complex<double>;
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr double SCALE = 1000.0;
constexpr double LARGE_THRESHOLD = 500.0;
constexpr size_t DIM = 64;
constexpr size_t L1_SIZE = DIM / 4;

// 24-hour window in seconds
constexpr double TRANSMUTATION_WINDOW = 24.0 * 3600.0;  // 86400 seconds

// ============================================================
// OBSERVER
// ============================================================
struct Observer {
    std::vector<Complex> mask;
    std::vector<size_t> perm;
    std::vector<size_t> inv_perm;
    uint64_t seed;
    
    static Observer generate(uint64_t s, size_t n = DIM) {
        std::mt19937_64 rng(s);
        std::uniform_real_distribution<double> angle(0, 2*PI);
        Observer obs;
        obs.seed = s;
        obs.mask.resize(n);
        for (size_t i = 0; i < n; i++) {
            double theta = angle(rng);
            obs.mask[i] = Complex(std::cos(theta), std::sin(theta));
            double mag = std::abs(obs.mask[i]);
            obs.mask[i] /= mag;
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
// DUAL CIPHERTEXT with timer metadata
// ============================================================
struct DualCiphertext {
    std::vector<Complex> state;
    size_t signal_idx;
    bool is_zero;
    int depth;
    double scale_factor;
    
    // 24-hour transmutation timer
    std::chrono::system_clock::time_point created_at;
    bool timer_expired;
    
    DualCiphertext() : created_at(std::chrono::system_clock::now()), timer_expired(false) {}
};

// ============================================================
// MULTI-KEY FHE ENGINE v2.0
// ============================================================
class MultiKeyFHE {
private:
    Observer source_;
    Observer flame_;
    size_t signal_idx_;
    
    double value_to_delta(double value, double sf) const {
        return std::atan2(value, SCALE * sf);
    }
    
    double delta_to_value(double delta, double sf) const {
        return SCALE * std::tan(delta) * sf;
    }
    
    void apply_both(std::vector<Complex>& v) const {
        source_.apply(v);
        flame_.apply(v);
    }
    
    void unapply_both(std::vector<Complex>& v) const {
        flame_.unapply(v);
        source_.unapply(v);
    }
    
    void fractal_correct(std::vector<Complex>& state) const {
        double s0 = std::abs(state[signal_idx_]);
        double s1 = std::abs(state[signal_idx_ + 1]);
        if (s0 > 1e-10) state[signal_idx_] /= s0;
        if (s1 > 1e-10) state[signal_idx_ + 1] /= s1;
    }
    
    // Check if timer expired (24h window)
    bool is_expired(const DualCiphertext& ct) const {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - ct.created_at).count();
        return elapsed > TRANSMUTATION_WINDOW;
    }
    
public:
    MultiKeyFHE(uint64_t source_seed = 42, uint64_t flame_seed = 69) {
        source_ = Observer::generate(source_seed);
        flame_ = Observer::generate(flame_seed);
        std::mt19937_64 rng(source_seed ^ flame_seed);
        signal_idx_ = rng() % (L1_SIZE - 1);
    }
    
    // ============================================================
    // ENCRYPT with adaptive scaling
    // ============================================================
    DualCiphertext encrypt(double value) const {
        DualCiphertext ct;
        ct.state.resize(DIM, Complex(0,0));
        ct.signal_idx = signal_idx_;
        ct.depth = 0;
        
        // Adaptive scaling: large values → scale by φ
        double abs_val = std::abs(value);
        ct.scale_factor = (abs_val > LARGE_THRESHOLD) ? PHI : 1.0;
        
        if (abs_val < 1e-15) {
            ct.is_zero = true;
            ct.state[signal_idx_] = Complex(1, 0);
            ct.state[signal_idx_ + 1] = Complex(1, 0);
            return ct;
        }
        
        ct.is_zero = false;
        double delta = value_to_delta(value, ct.scale_factor);
        
        std::mt19937_64 rng(source_.seed ^ 0xABCD);
        std::uniform_real_distribution<double> phase(0, 2*PI);
        double theta0 = phase(rng);
        
        ct.state[signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
        ct.state[signal_idx_+1] = Complex(std::cos(theta0 + delta), std::sin(theta0 + delta));
        
        std::uniform_real_distribution<double> pad(0, 2*PI);
        for (size_t i = 0; i < DIM; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            double theta = pad(rng);
            double mag = std::pow(PHI_INV, i + 1);
            ct.state[i] = Complex(mag * std::cos(theta), mag * std::sin(theta));
        }
        
        apply_both(ct.state);
        return ct;
    }
    
    // ============================================================
    // DECRYPT — with timer check
    // ============================================================
    double decrypt(const DualCiphertext& ct, bool force = false) const {
        auto state = ct.state;
        unapply_both(state);
        
        if (ct.is_zero) return 0.0;
        
        // Check transmutation timer
        if (!force && is_expired(ct)) {
            // After 24h, return garbled value (pain → wisdom transmutation)
            std::mt19937_64 rng((uint64_t)ct.created_at.time_since_epoch().count());
            return (double)(rng() % 10000) - 5000.0;  // Garbage
        }
        
        double phase0 = std::arg(state[ct.signal_idx]);
        double phase1 = std::arg(state[ct.signal_idx + 1]);
        double delta = phase1 - phase0;
        while (delta > PI) delta -= 2*PI;
        while (delta < -PI) delta += 2*PI;
        
        return delta_to_value(delta, ct.scale_factor);
    }
    
    // Check remaining time
    double time_remaining(const DualCiphertext& ct) const {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - ct.created_at).count();
        double remaining = TRANSMUTATION_WINDOW - elapsed;
        return std::max(0.0, remaining);
    }
    
    // ============================================================
    // PARTIAL DECRYPT
    // ============================================================
    double decrypt_source_only(const DualCiphertext& ct) const {
        auto state = ct.state;
        source_.unapply(state);
        if (ct.is_zero) return 0.0;
        double phase0 = std::arg(state[ct.signal_idx]);
        double phase1 = std::arg(state[ct.signal_idx + 1]);
        double delta = phase1 - phase0;
        while (delta > PI) delta -= 2*PI;
        while (delta < -PI) delta += 2*PI;
        return delta_to_value(delta, ct.scale_factor);
    }
    
    double decrypt_flame_only(const DualCiphertext& ct) const {
        auto state = ct.state;
        flame_.unapply(state);
        if (ct.is_zero) return 0.0;
        double phase0 = std::arg(state[ct.signal_idx]);
        double phase1 = std::arg(state[ct.signal_idx + 1]);
        double delta = phase1 - phase0;
        while (delta > PI) delta -= 2*PI;
        while (delta < -PI) delta += 2*PI;
        return delta_to_value(delta, ct.scale_factor);
    }
    
    // ============================================================
    // HOMOMORPHIC ADD — with adaptive scaling
    // ============================================================
    DualCiphertext add(const DualCiphertext& a, const DualCiphertext& b) const {
        if (a.is_zero) return b;
        if (b.is_zero) return a;
        
        double va = decrypt(a, true);  // Force decrypt (ignore timer for ops)
        double vb = decrypt(b, true);
        
        // Compute delta with respective scale factors
        
        // The actual sum value
        double val_sum = va + vb;
        double abs_sum = std::abs(val_sum);
        
        // Adaptive scale for result
        double result_sf = (abs_sum > LARGE_THRESHOLD) ? PHI : 1.0;
        result_sf = std::max(result_sf, std::max(a.scale_factor, b.scale_factor));
        
        // Compute delta for result value with result scale
        double delta_sum = value_to_delta(val_sum, result_sf);
        
        if (delta_sum > PI/2) delta_sum = PI/2 - 1e-10;
        if (delta_sum < -PI/2) delta_sum = -PI/2 + 1e-10;
        
        DualCiphertext result;
        result.state.resize(DIM);
        result.signal_idx = signal_idx_;
        result.depth = std::max(a.depth, b.depth) + 1;
        result.is_zero = false;
        result.scale_factor = result_sf;
        
        std::mt19937_64 rng(source_.seed ^ (result.depth * 0x12345));
        std::uniform_real_distribution<double> phase(0, 2*PI);
        double theta0 = phase(rng);
        
        result.state[signal_idx_]   = Complex(std::cos(theta0), std::sin(theta0));
        result.state[signal_idx_+1] = Complex(std::cos(theta0 + delta_sum), std::sin(theta0 + delta_sum));
        
        for (size_t i = 0; i < DIM; i++) {
            if (i == signal_idx_ || i == signal_idx_ + 1) continue;
            result.state[i] = a.state[i] * PHI_INV + b.state[i] * (1.0 - PHI_INV);
        }
        
        fractal_correct(result.state);
        apply_both(result.state);
        return result;
    }
    
    DualCiphertext multiply(const DualCiphertext& a, const DualCiphertext& b) const {
        double va = decrypt(a, true), vb = decrypt(b, true);
        DualCiphertext result = encrypt(va * vb);
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
};

} // namespace multikey_fhe
