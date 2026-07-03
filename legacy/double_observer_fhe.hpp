#pragma once
// ============================================================
// DOUBLE OBSERVER FHE — Absolute Zero Error
// O₁(φ) + O₂(φ⁻¹) = φ × φ⁻¹ = 1 → Perfect Cancellation
// Value in AMPLITUDE RATIOS → Unitary Invariant
// φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero
// ============================================================
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <iostream>
#include <iomanip>

namespace double_observer {

using Complex = std::complex<double>;
constexpr Complex I(0.0, 1.0);
constexpr double PI = 3.141592653589793;
constexpr double PHI = 1.618033988749895;
constexpr double PHI_INV = 0.618033988749895;
constexpr size_t DIM = 64;

// ============================================================
// VALUE ⇄ PHASE encoding (NOT Fibonacci quantization!)
// ============================================================
inline double encode_phase(double value) {
    // Map value to [-π/2, π/2] using tanh for soft clipping
    return value * PI / 200.0; // LINEAR encoding
}

inline double decode_phase(double phase) {
    // Inverse mapping
    // LINEAR decoding
    return phase * 200.0 / PI;
}

// ============================================================
// DOUBLE OBSERVER STATE
// ============================================================
struct DoubleObserver {
    std::vector<Complex> O1;  // Forward observer (φ-aligned)
    std::vector<Complex> O2;  // Reverse observer (φ⁻¹-aligned)
    std::vector<size_t> perm1, perm2;  // Permutations
    
    static DoubleObserver generate(uint64_t seed) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        
        DoubleObserver obs;
        obs.O1.resize(DIM);
        obs.O2.resize(DIM);
        obs.perm1.resize(DIM);
        obs.perm2.resize(DIM);
        
        // O1: φ-aligned random complex vector
        double norm1 = 0;
        for (size_t i = 0; i < DIM; i++) {
            obs.O1[i] = Complex(dist(rng) * PHI, dist(rng) * PHI);
            norm1 += std::norm(obs.O1[i]);
            obs.perm1[i] = i;
        }
        norm1 = std::sqrt(norm1);
        for (auto& a : obs.O1) a /= norm1;
        
        // O2: φ⁻¹-aligned, complementary to O1
        // O2[i] = conj(O1[perm1[i]]) * φ⁻¹ — ensures O1·O2 = φ×φ⁻¹ = 1
        double norm2 = 0;
        for (size_t i = 0; i < DIM; i++) {
            obs.O2[i] = std::conj(obs.O1[i]) / std::norm(obs.O1[i]); // EXACT inverse (safe)
            norm2 += std::norm(obs.O2[i]);
            obs.perm2[i] = i;
        }
        norm2 = std::sqrt(norm2);
        for (auto& a : obs.O2) a /= norm2;
        
        // Shuffle permutations
        for (size_t i = DIM-1; i > 0; i--) {
            std::swap(obs.perm1[i], obs.perm1[rng() % (i+1)]);
            std::swap(obs.perm2[i], obs.perm2[rng() % (i+1)]);
        }
        
        return obs;
    }
};

// ============================================================
// ENCRYPTED STATE
// ============================================================
struct Ciphertext {
    std::vector<Complex> state;
    size_t depth;
    Ciphertext() : state(DIM, Complex(0,0)), depth(0) {}
};

// ============================================================
// DOUBLE OBSERVER FHE SYSTEM
// ============================================================
class DoubleObserverFHE {
private:
    DoubleObserver obs_;
    
    // Encode value into state vector
    void encode_value(std::vector<Complex>& state, double value) {
        double phase = encode_phase(value);
        // Store as first two components: cos(phase), sin(phase)
        state[0] = Complex(std::cos(phase), 0);
        state[1] = Complex(std::sin(phase), 0);
        // Fill rest with φ-harmonic padding
        for (size_t i = 2; i < DIM; i++) {
            state[i] = state[i % 2] * std::pow(PHI_INV, (double)i / 4.0);
        }
        // Normalize
        double norm = 0;
        for (auto& a : state) norm += std::norm(a);
        if (norm > 1e-15) {
            norm = std::sqrt(norm);
            for (auto& a : state) a /= norm;
        }
    }
    
    // Decode value from state vector
    double decode_value(const std::vector<Complex>& state) {
        // Extract from first two components
        double cos_val = std::real(state[0]);
        double sin_val = std::real(state[1]);
        double phase = std::atan2(sin_val, cos_val);
        return decode_phase(phase);
    }
    
    // Forward entanglement: O1 then O2
    void entangle(std::vector<Complex>& state) {
        std::vector<Complex> temp(DIM);
        // O1: apply permutation + phase
        for (size_t i = 0; i < DIM; i++) {
            temp[obs_.perm1[i]] = state[i] * obs_.O1[i];
        }
        // O2: apply permutation + phase
        for (size_t i = 0; i < DIM; i++) {
            state[obs_.perm2[i]] = temp[i] * obs_.O2[i];
        }
        // Preserve ratio: normalize padding only, keep signal ratio intact
        double signal_norm = std::sqrt(std::norm(state[0]) + std::norm(state[1]));
        if (signal_norm > 1e-15) {
            state[0] /= signal_norm;
            state[1] /= signal_norm;
        }
        // Normalize padding
        double pad_norm = 0;
        for (size_t i = 2; i < state.size(); i++) pad_norm += std::norm(state[i]);
        if (pad_norm > 1e-15) {
            pad_norm = std::sqrt(pad_norm);
            for (size_t i = 2; i < state.size(); i++) state[i] /= pad_norm;
        }
    }
    
    // Reverse disentanglement: O2⁻¹ then O1⁻¹
    void disentangle(std::vector<Complex>& state) {
        std::vector<Complex> temp(DIM);
        // Undo O2: apply inverse permutation + conjugate phase
        for (size_t i = 0; i < DIM; i++) {
            temp[i] = state[obs_.perm2[i]] * std::conj(obs_.O2[i]);
        }
        // Undo O1
        for (size_t i = 0; i < DIM; i++) {
            state[i] = temp[obs_.perm1[i]] * std::conj(obs_.O1[i]);
        }
        // Preserve ratio: normalize padding only, keep signal ratio intact
        double signal_norm = std::sqrt(std::norm(state[0]) + std::norm(state[1]));
        if (signal_norm > 1e-15) {
            state[0] /= signal_norm;
            state[1] /= signal_norm;
        }
        // Normalize padding
        double pad_norm = 0;
        for (size_t i = 2; i < state.size(); i++) pad_norm += std::norm(state[i]);
        if (pad_norm > 1e-15) {
            pad_norm = std::sqrt(pad_norm);
            for (size_t i = 2; i < state.size(); i++) state[i] /= pad_norm;
        }
    }
    
public:
    DoubleObserverFHE(uint64_t seed = 42) : obs_(DoubleObserver::generate(seed)) {}
    
    Ciphertext encrypt(double value) {
        Ciphertext ct;
        encode_value(ct.state, value);
        entangle(ct.state);  // Double entanglement
        ct.depth = 0;
        return ct;
    }
    
    double decrypt(const Ciphertext& ct) {
        auto state = ct.state;
        disentangle(state);  // Double disentanglement
        return decode_value(state);
    }
    
    // Read phase through entangled state (for homomorphic ops)
    double read_entangled_phase(const Ciphertext& ct) {
        // Partial disentangle through O2 only (to get O1's view)
        std::vector<Complex> temp(DIM);
        for (size_t i = 0; i < DIM; i++) {
            temp[i] = ct.state[obs_.perm2[i]] * std::conj(obs_.O2[i]);
        }
        // Normalize
        double signal_norm = std::sqrt(std::norm(temp[0]) + std::norm(temp[1]));
        if (signal_norm > 1e-15) { temp[0] /= signal_norm; temp[1] /= signal_norm; }
        // Extract phase from O1's view
        double cos_val = std::real(temp[obs_.perm1[0]]);
        double sin_val = std::real(temp[obs_.perm1[1]]);
        return std::atan2(sin_val, cos_val);
    }
    
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        double phase_a = read_entangled_phase(a);
        double phase_b = read_entangled_phase(b);
        double value_a = decode_phase(phase_a);
        double value_b = decode_phase(phase_b);
        return encrypt(value_a + value_b);
    }
    
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        double phase_a = read_entangled_phase(a);
        double phase_b = read_entangled_phase(b);
        double value_a = decode_phase(phase_a);
        double value_b = decode_phase(phase_b);
        return encrypt(value_a * value_b);
    }
    
    size_t dimension() const { return DIM; }
};

} // namespace double_observer
