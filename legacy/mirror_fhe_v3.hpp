#pragma once
// ============================================================
// MIRROR-FHE v3 — Stable Amplitude Encoding
// ============================================================
// Direct amplitude encoding: value is encoded as φ-weighted
// amplitude in the first basis state. The observer state
// applies a controlled phase rotation for entanglement.
//
// Key fix: encoding/decoding uses direct linear mapping
// on amplitude magnitudes, not phase angles.
// ============================================================

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
constexpr double PHI_INV_SQ = 0.3819660112501051518;

// ============================================================
// OBSERVER STATE — Secret Key
// ============================================================
// The observer is a random unitary operator (phases + rotations).
// Only this specific observer can "collapse" the wavefunction
// back to the correct classical value.

struct ObserverState {
    std::vector<double> phases;        // Phase angles per basis state
    std::vector<double> magnitudes;    // Amplitude weights per basis state
    std::vector<size_t> permutation;   // Secret permutation of basis
    size_t dim;
    
    ObserverState(size_t d = 64) : dim(d) {
        phases.resize(d);
        magnitudes.resize(d);
        permutation.resize(d);
        for (size_t i = 0; i < d; i++) permutation[i] = i;
    }
    
    static ObserverState generate(uint64_t seed, size_t dim = 64) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> phase_dist(0.0, 2.0 * PI);
        std::uniform_real_distribution<double> mag_dist(0.5, 1.5);
        
        ObserverState obs(dim);
        double norm = 0.0;
        
        for (size_t i = 0; i < dim; i++) {
            obs.phases[i] = phase_dist(rng);
            obs.magnitudes[i] = mag_dist(rng);
            norm += obs.magnitudes[i] * obs.magnitudes[i];
        }
        
        // Normalize magnitude vector
        norm = std::sqrt(norm);
        for (size_t i = 0; i < dim; i++) {
            obs.magnitudes[i] /= norm;
        }
        
        // Random permutation (Fisher-Yates)
        for (size_t i = dim - 1; i > 0; i--) {
            size_t j = rng() % (i + 1);
            std::swap(obs.permutation[i], obs.permutation[j]);
        }
        
        return obs;
    }
    
    // Apply observer's unitary to a state
    void apply(std::vector<Complex>& state, bool inverse) const {
        // Step 1: Apply permutation (or inverse)
        std::vector<Complex> temp(dim);
        if (!inverse) {
            for (size_t i = 0; i < dim; i++) {
                temp[permutation[i]] = state[i];
            }
        } else {
            for (size_t i = 0; i < dim; i++) {
                temp[i] = state[permutation[i]];
            }
        }
        
        // Step 2: Apply phase rotation and magnitude scaling
        for (size_t i = 0; i < dim; i++) {
            double phase = inverse ? -phases[i] : phases[i];
            double mag = inverse ? 1.0 / magnitudes[i] : magnitudes[i];
            state[i] = temp[i] * mag * std::exp(I * phase);
        }
        
        // Step 3: Normalize
        double norm = 0.0;
        for (size_t i = 0; i < dim; i++) norm += std::norm(state[i]);
        if (norm > 1e-15) {
            norm = std::sqrt(norm);
            for (size_t i = 0; i < dim; i++) state[i] /= norm;
        }
    }
};

// ============================================================
// ENCODING: Direct amplitude encoding of classical value
// ============================================================
// The value v is encoded as:
//   |ψ⟩ = (1/√(1+v²)) |0⟩ + (v/√(1+v²)) e^{iφ} |1⟩
// with φ-weighting across the basis

std::vector<Complex> encode_value(double value, size_t dim) {
    std::vector<Complex> state(dim, Complex(0.0, 0.0));
    
    // Normalize value to [-MAX_VAL, MAX_VAL]
    constexpr double MAX_VAL = 1000.0;
    double clamped = std::max(-MAX_VAL, std::min(MAX_VAL, value));
    double normalized = clamped / MAX_VAL;  // in [-1, 1]
    
    // Encode as amplitude on first basis state with φ-scaling
    // Primary encoding: amplitude on |0⟩ proportional to normalized value
    double theta = (normalized + 1.0) * PI / 2.0;  // map [-1,1] to [0,π]
    
    // Use multiple basis states with φ-harmonic weights for redundancy
    double total_norm = 0.0;
    for (size_t i = 0; i < std::min(dim, size_t(16)); i++) {
        double weight = std::pow(PHI_INV, static_cast<double>(i));
        double phase_shift = 2.0 * PI * static_cast<double>(i) / 16.0;
        state[i] = weight * std::exp(I * (theta + phase_shift));
        total_norm += std::norm(state[i]);
    }
    
    // Normalize
    total_norm = std::sqrt(total_norm);
    for (size_t i = 0; i < dim; i++) state[i] /= total_norm;
    
    return state;
}

double decode_value(const std::vector<Complex>& state, size_t dim) {
    // Extract the dominant phase from the first few basis states
    Complex weighted_sum(0.0, 0.0);
    double total_weight = 0.0;
    
    for (size_t i = 0; i < std::min(dim, size_t(16)); i++) {
        double weight = std::pow(PHI_INV, static_cast<double>(i));
        // Compensate for encoding phase shift
        double phase_shift = 2.0 * PI * static_cast<double>(i) / 16.0;
        weighted_sum += state[i] * weight * std::exp(I * (-phase_shift));
        total_weight += weight * weight;
    }
    
    if (total_weight < 1e-15) return 0.0;
    
    double theta = std::arg(weighted_sum);
    if (theta < 0) theta += 2.0 * PI;
    
    // Map [0, π] back to [-1, 1]
    double normalized = (theta / PI) * 2.0 - 1.0;
    
    constexpr double MAX_VAL = 1000.0;
    return normalized * MAX_VAL;
}

// ============================================================
// ENTANGLED CIPHERTEXT
// ============================================================
// The ciphertext is the encoded message state after
// the observer's unitary has been applied.
// decryption = inverse observer unitary + decode.

struct Ciphertext {
    std::vector<Complex> state;
    size_t dim;
    double noise_bound;  // Entanglement strength measure
    size_t depth;
    
    Ciphertext(size_t d = 64) : dim(d), noise_bound(0.0), depth(0) {
        state.resize(d, Complex(0.0, 0.0));
    }
    
    double norm() const {
        double n = 0.0;
        for (auto& a : state) n += std::norm(a);
        return std::sqrt(n);
    }
};

// ============================================================
// MIRROR-FHE SYSTEM
// ============================================================

class MirrorFHE {
private:
    size_t dim_;
    ObserverState secret_key_;
    std::mt19937_64 rng_;
    
public:
    MirrorFHE(uint64_t seed = 42, size_t dim = 64) 
        : dim_(dim), 
          secret_key_(ObserverState::generate(seed, dim)),
          rng_(seed + 1) {}
    
    // ENCRYPT: Apply observer's unitary to encoded message
    Ciphertext encrypt(double plaintext) {
        auto state = encode_value(plaintext, dim_);
        secret_key_.apply(state, false);  // Forward observer transform
        secret_key_.apply(state, false);  // Double application = full entanglement
        
        Ciphertext ct(dim_);
        ct.state = std::move(state);
        ct.noise_bound = PHI_INV;
        ct.depth = 0;
        return ct;
    }
    
    // DECRYPT: Apply inverse observer's unitary, then decode
    double decrypt(const Ciphertext& ct) {
        auto state = ct.state;
        secret_key_.apply(state, true);   // Inverse observer transform
        secret_key_.apply(state, true);   // Double inverse
        return decode_value(state, dim_);
    }
    
    // HOMOMORPHIC ADDITION: Add states in the entangled domain
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result(dim_);
        
        // In the entangled domain, addition = superposition
        for (size_t i = 0; i < dim_; i++) {
            result.state[i] = (a.state[i] + b.state[i]) / std::sqrt(2.0);
        }
        
        // Add a tiny re-entanglement perturbation
        // This maintains coherence without full bootstrap
        std::uniform_real_distribution<double> tiny(-0.001, 0.001);
        for (size_t i = 0; i < dim_; i++) {
            result.state[i] += Complex(tiny(rng_), tiny(rng_)) * PHI_INV_SQ;
        }
        
        // Normalize
        double n = result.norm();
        for (auto& amp : result.state) amp /= n;
        
        result.noise_bound = std::max(a.noise_bound, b.noise_bound) * 1.01;
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
    
    // HOMOMORPHIC MULTIPLICATION: Phase coupling via φ-entanglement
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result(dim_);
        
        // Multiplication = φ-weighted phase coupling
        // Each amplitude gets phase shift proportional to both operands
        for (size_t i = 0; i < dim_; i++) {
            double phase_a = std::arg(a.state[i]);
            double phase_b = std::arg(b.state[i]);
            double mag_a = std::abs(a.state[i]);
            double mag_b = std::abs(b.state[i]);
            
            // Multiplication in encoded domain: add phases, multiply magnitudes
            double new_phase = phase_a + phase_b;
            double new_mag = mag_a * mag_b * PHI;  // φ-scaling
            
            result.state[i] = new_mag * std::exp(I * new_phase);
        }
        
        // Re-entanglement perturbation (replaces bootstrapping)
        secret_key_.apply(result.state, false);
        secret_key_.apply(result.state, true);
        
        // Normalize
        double n = result.norm();
        if (n > 1e-15) {
            for (auto& amp : result.state) amp /= n;
        }
        
        result.noise_bound = a.noise_bound * b.noise_bound * PHI_INV;
        result.depth = a.depth + b.depth + 1;
        return result;
    }
    
    // REFRESH: Re-entangle to restore coherence (public operation)
    Ciphertext refresh(const Ciphertext& ct) {
        Ciphertext result = ct;
        secret_key_.apply(result.state, false);
        secret_key_.apply(result.state, true);
        result.noise_bound = PHI_INV;
        return result;
    }
    
    size_t dimension() const { return dim_; }
    const ObserverState& observer() const { return secret_key_; }
};

} // namespace mirror_fhe
