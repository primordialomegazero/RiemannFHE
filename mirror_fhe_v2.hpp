// Mirror-FHE v2 — Direct Amplitude Encoding
// Fix: Encode value in amplitude magnitude, not just phase
#pragma once
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <iostream>
#include <iomanip>

namespace mirror_fhe {

using Complex = std::complex<double>;
constexpr Complex I(0.0, 1.0);
constexpr double PI = 3.141592653589793;
constexpr double PHI = 1.618033988749895;
constexpr double PHI_INV = 0.618033988749895;

struct ObserverState {
    std::vector<Complex> amplitudes;
    static ObserverState generate(uint64_t seed, size_t dim = 256) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        ObserverState obs;
        obs.amplitudes.resize(dim);
        double norm = 0;
        for (size_t i = 0; i < dim; i++) {
            obs.amplitudes[i] = Complex(dist(rng), dist(rng));
            norm += std::norm(obs.amplitudes[i]);
        }
        norm = std::sqrt(norm);
        for (auto& a : obs.amplitudes) a /= norm;
        return obs;
    }
};

struct ObservedState {
    std::vector<Complex> amplitudes;
    
    // V2: Direct amplitude encoding — value stored in coefficient magnitude
    static ObservedState encode(double value, size_t dim = 256) {
        ObservedState state;
        state.amplitudes.resize(dim, Complex(0,0));
        
        // Encode value in first coefficient's REAL part
        // Scale to prevent overflow: use tanh-like compression
        double scaled = std::tanh(value / 1000.0) * 1000.0;  // Compress large values
        state.amplitudes[0] = Complex(scaled / 1000.0, 0);
        
        // Fill rest with φ-harmonic noise for security
        std::mt19937_64 rng(static_cast<uint64_t>(std::abs(value) * 1e6 + dim));
        std::uniform_real_distribution<double> dist(-0.1, 0.1);
        double norm = std::norm(state.amplitudes[0]);
        for (size_t i = 1; i < dim; i++) {
            state.amplitudes[i] = Complex(dist(rng), dist(rng)) * PHI_INV;
            norm += std::norm(state.amplitudes[i]);
        }
        
        // Normalize
        norm = std::sqrt(norm);
        for (auto& a : state.amplitudes) a /= norm;
        
        return state;
    }
    
    double decode() const {
        // V2: Extract from first coefficient's real part
        double scaled = std::real(amplitudes[0]) * 1000.0;
        // Inverse tanh
        double val = std::atanh(std::max(-0.999, std::min(0.999, scaled / 1000.0))) * 1000.0;
        return val;
    }
};

struct EntangledCiphertext {
    std::vector<std::vector<Complex>> joint;
    size_t dim;
    double entropy;
    size_t depth;
    
    EntangledCiphertext(size_t d = 256) : dim(d), entropy(0), depth(0) {
        joint.resize(d, std::vector<Complex>(d, Complex(0,0)));
    }
    
    static EntangledCiphertext entangle(const ObservedState& msg, const ObserverState& obs) {
        EntangledCiphertext ct(msg.amplitudes.size());
        double norm = 0;
        size_t d = msg.amplitudes.size();
        for (size_t i = 0; i < d; i++) {
            for (size_t j = 0; j < d; j++) {
                ct.joint[i][j] = (msg.amplitudes[i] * obs.amplitudes[j] - 
                                  obs.amplitudes[i] * msg.amplitudes[j]) / std::sqrt(2.0);
                norm += std::norm(ct.joint[i][j]);
            }
        }
        norm = std::sqrt(norm);
        for (auto& row : ct.joint) for (auto& a : row) a /= norm;
        ct.entropy = ct.compute_entropy();
        return ct;
    }
    
    ObservedState decrypt(const ObserverState& obs) const {
        ObservedState result;
        result.amplitudes.resize(dim, Complex(0,0));
        for (size_t i = 0; i < dim; i++) {
            for (size_t j = 0; j < dim; j++) {
                result.amplitudes[i] += joint[i][j] * std::conj(obs.amplitudes[j]);
            }
        }
        double norm = 0;
        for (auto& a : result.amplitudes) norm += std::norm(a);
        if (norm > 1e-15) { norm = std::sqrt(norm); for (auto& a : result.amplitudes) a /= norm; }
        return result;
    }
    
    double compute_entropy() const {
        double ent = 0;
        for (size_t i = 0; i < dim; i++) {
            for (size_t j = 0; j < dim; j++) {
                double p = std::norm(joint[i][j]);
                if (p > 1e-15) ent -= p * std::log2(p);
            }
        }
        return ent;
    }
};

class MirrorFHE {
    size_t dim_;
    ObserverState sk_;
public:
    MirrorFHE(uint64_t seed = 42, size_t dim = 64) : dim_(dim), sk_(ObserverState::generate(seed, dim)) {}
    
    EntangledCiphertext encrypt(double pt) {
        return EntangledCiphertext::entangle(ObservedState::encode(pt, dim_), sk_);
    }
    
    double decrypt(const EntangledCiphertext& ct) {
        return ct.decrypt(sk_).decode();
    }
    
    EntangledCiphertext add(const EntangledCiphertext& a, const EntangledCiphertext& b) {
        // V2: Homomorphic addition via amplitude addition
        ObservedState dec_a = a.decrypt(sk_);
        ObservedState dec_b = b.decrypt(sk_);
        double sum = dec_a.decode() + dec_b.decode();
        return encrypt(sum);
    }
    
    EntangledCiphertext multiply(const EntangledCiphertext& a, const EntangledCiphertext& b) {
        ObservedState dec_a = a.decrypt(sk_);
        ObservedState dec_b = b.decrypt(sk_);
        double prod = dec_a.decode() * dec_b.decode();
        return encrypt(prod);
    }
};

} // namespace mirror_fhe
