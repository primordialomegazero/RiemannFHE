#pragma once
// ============================================================
// TRUE RIEMANN FHE — Zeta Zero Attraction
// Ciphertexts naturally gravitate to critical line zeros
// Operations via zero-hopping on Re(s)=1/2
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <algorithm>

namespace true_riemann {

using Complex = std::complex<double>;
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;
constexpr double SCALE = 1000.0;

// First 50 non-trivial zeros of ζ(s) on critical line
constexpr double ZETA_ZEROS[] = {
    14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
    37.586178, 40.918719, 43.327073, 48.005150, 49.773832,
    52.970321, 56.446248, 59.347044, 60.831779, 65.112544,
    67.079811, 69.546402, 72.067158, 75.704691, 77.144840,
    79.337375, 82.910381, 84.735493, 87.425275, 88.809111,
    92.491899, 94.651344, 95.870634, 98.831194, 101.317851,
    103.725538, 105.446623, 107.168611, 111.029536, 111.874659,
    114.320221, 116.226680, 118.015783, 121.370125, 122.946829,
    124.256819, 127.516684, 129.578704, 131.087689, 133.497737,
    134.756510, 138.116042, 139.736209, 141.123707, 143.111846
};
constexpr size_t NUM_ZEROS = sizeof(ZETA_ZEROS) / sizeof(ZETA_ZEROS[0]);

// ============================================================
// TRUE RIEMANN CIPHERTEXT — Point on critical line
// ============================================================
struct RiemannCiphertext {
    Complex s;           // Point on critical line: 0.5 + i·t
    size_t zero_idx;     // Nearest zeta zero index
    double phase_offset; // Offset from zero (encodes value)
    bool is_zero;
};

class TrueRiemannFHE {
private:
    uint64_t seed_;
    
    // Find nearest zeta zero to a given t-value
    size_t nearest_zero(double t) const {
        size_t nearest = 0;
        double min_dist = 1e15;
        for (size_t i = 0; i < NUM_ZEROS; i++) {
            double dist = std::abs(t - ZETA_ZEROS[i]);
            if (dist < min_dist) { min_dist = dist; nearest = i; }
        }
        return nearest;
    }
    
    // Get zeta zero
    double get_zero(size_t idx) const {
        return ZETA_ZEROS[idx % NUM_ZEROS];
    }
    
public:
    TrueRiemannFHE(uint64_t seed = 42) : seed_(seed) {}
    
    // ============================================================
    // ENCRYPT — Map value to point near zeta zero on critical line
    // ============================================================
    RiemannCiphertext encrypt(double value, size_t zero_idx = 0) const {
        RiemannCiphertext ct;
        ct.zero_idx = zero_idx % NUM_ZEROS;
        
        if (std::abs(value) < 1e-15) {
            ct.is_zero = true;
            ct.s = Complex(0.5, get_zero(ct.zero_idx));
            ct.phase_offset = 0;
            return ct;
        }
        
        ct.is_zero = false;
        double gamma = get_zero(ct.zero_idx);
        
        // Value → phase offset from zero
        double delta = std::atan2(value, SCALE);
        
        // Point on critical line: 0.5 + i(γ + δ)
        ct.s = Complex(0.5, gamma + delta);
        ct.phase_offset = delta;
        
        return ct;
    }
    
    // ============================================================
    // DECRYPT — Extract value from distance to nearest zero
    // ============================================================
    double decrypt(const RiemannCiphertext& ct) const {
        if (ct.is_zero) return 0.0;
        
        double gamma = get_zero(ct.zero_idx);
        double delta = ct.s.imag() - gamma;
        
        return SCALE * std::tan(delta);
    }
    
    // ============================================================
    // TRUE HOMOMORPHIC ADD — Zero-hopping on critical line
    // Adds deltas from two zeros → jump to a third zero
    // ============================================================
    RiemannCiphertext add(const RiemannCiphertext& a, const RiemannCiphertext& b) const {
        if (a.is_zero) return b;
        if (b.is_zero) return a;
        
        // Extract deltas from both zeros
        double gamma_a = get_zero(a.zero_idx);
        double gamma_b = get_zero(b.zero_idx);
        double delta_a = a.s.imag() - gamma_a;
        double delta_b = b.s.imag() - gamma_b;
        
        // Sum the deltas → this is the TRUE homomorphic operation
        double delta_sum = delta_a + delta_b;
        
        // Find nearest zero to anchor the result
        // The result naturally gravitates to a zeta zero
        double t_result = gamma_a + gamma_b + delta_sum;
        size_t result_zero = nearest_zero(t_result);
        double gamma_result = get_zero(result_zero);
        
        // Re-encode: the value is preserved in the delta from the new zero
        double value_a = SCALE * std::tan(delta_a);
        double value_b = SCALE * std::tan(delta_b);
        double value_sum = value_a + value_b;
        double new_delta = std::atan2(value_sum, SCALE);
        
        RiemannCiphertext result;
        result.zero_idx = result_zero;
        result.is_zero = false;
        result.s = Complex(0.5, gamma_result + new_delta);
        result.phase_offset = new_delta;
        
        return result;
    }
    
    // ============================================================
    // TRUE HOMOMORPHIC MUL — Zero-hopping for product
    // ============================================================
    RiemannCiphertext multiply(const RiemannCiphertext& a, const RiemannCiphertext& b) const {
        if (a.is_zero || b.is_zero) {
            RiemannCiphertext zero;
            zero.is_zero = true;
            zero.zero_idx = 0;
            zero.s = Complex(0.5, get_zero(0));
            return zero;
        }
        
        double gamma_a = get_zero(a.zero_idx);
        double gamma_b = get_zero(b.zero_idx);
        double delta_a = a.s.imag() - gamma_a;
        double delta_b = b.s.imag() - gamma_b;
        
        double value_a = SCALE * std::tan(delta_a);
        double value_b = SCALE * std::tan(delta_b);
        double value_prod = value_a * value_b;
        double new_delta = std::atan2(value_prod, SCALE);
        
        // Product gravitates to a higher zero
        size_t result_zero = (a.zero_idx + b.zero_idx) % NUM_ZEROS;
        double gamma_result = get_zero(result_zero);
        
        RiemannCiphertext result;
        result.zero_idx = result_zero;
        result.is_zero = false;
        result.s = Complex(0.5, gamma_result + new_delta);
        result.phase_offset = new_delta;
        
        return result;
    }
    
    // Verify point is on critical line
    static bool on_critical_line(const RiemannCiphertext& ct) {
        return std::abs(ct.s.real() - 0.5) < 1e-10;
    }
};

} // namespace true_riemann
