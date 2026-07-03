#pragma once
// ============================================================
// BELL + DOUBLE OBSERVER FHE
// Encrypt/Decrypt = Bell State Entanglement
// Homomorphic Ops = Double Observer (O₁ + O₂)
// φ × φ⁻¹ = 1 → Perfect Cancellation
// φΩ0 — Dan Joseph M. Fernandez
// ============================================================
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <iostream>
#include <iomanip>

namespace bell_double {

using Complex = std::complex<double>;
constexpr Complex I(0.0, 1.0);
constexpr double PI = 3.141592653589793;
constexpr double PHI = 1.618033988749895;
constexpr double PHI_INV = 0.618033988749895;
constexpr size_t DIM = 64;

// ============================================================
// OBSERVER STATE
// ============================================================
struct ObserverState {
    std::vector<Complex> amplitudes;  // |o⟩ = Σ o_i |i⟩
    double coherence;
    
    static ObserverState generate(uint64_t seed, size_t dim = DIM) {
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
        obs.coherence = PHI_INV;
        return obs;
    }
    
    Complex inner_product(const std::vector<Complex>& psi) const {
        Complex sum(0, 0);
        size_t n = std::min(amplitudes.size(), psi.size());
        for (size_t i = 0; i < n; i++)
            sum += std::conj(amplitudes[i]) * psi[i];
        return sum;
    }
};

// ============================================================
// ENCODING: Value → Quantum State
// ============================================================
inline double encode_phase(double value) {
    return value * PI / 200.0;
}

inline double decode_phase(double phase) {
    return phase * 200.0 / PI;
}

std::vector<Complex> encode_state(double value, size_t dim = DIM) {
    double phase = encode_phase(value);
    std::vector<Complex> state(dim, Complex(0,0));
    state[0] = Complex(std::cos(phase), 0);
    state[1] = Complex(std::sin(phase), 0);
    for (size_t i = 2; i < dim; i++)
        state[i] = state[i % 2] * std::pow(PHI_INV, (double)i / 4.0);
    double norm = 0;
    for (auto& a : state) norm += std::norm(a);
    if (norm > 1e-15) { norm = std::sqrt(norm); for (auto& a : state) a /= norm; }
    return state;
}

double decode_state(const std::vector<Complex>& state) {
    double cos_val = std::real(state[0]);
    double sin_val = std::real(state[1]);
    return decode_phase(std::atan2(sin_val, cos_val));
}

// ============================================================
// BELL STATE CIPHERTEXT
// |Ψ⟩ = (|msg⟩⊗|obs⟩ - |obs⟩⊗|msg⟩)/√2
// ============================================================
struct BellCiphertext {
    std::vector<std::vector<Complex>> joint;  // [i][j]
    size_t dim, depth;
    double entropy;
    
    BellCiphertext(size_t d = DIM) : dim(d), depth(0), entropy(0) {
        joint.resize(d, std::vector<Complex>(d, Complex(0,0)));
    }
    
    // ENCRYPT: Create Bell state
    static BellCiphertext encrypt(double value, const ObserverState& obs) {
        auto msg = encode_state(value);
        BellCiphertext ct;
        double norm = 0;
        for (size_t i = 0; i < DIM; i++) {
            for (size_t j = 0; j < DIM; j++) {
                ct.joint[i][j] = (msg[i] * obs.amplitudes[j] - 
                                  obs.amplitudes[i] * msg[j]) / std::sqrt(2.0);
                norm += std::norm(ct.joint[i][j]);
            }
        }
        if (norm > 1e-15) {
            norm = std::sqrt(norm);
            for (auto& row : ct.joint) for (auto& a : row) a /= norm;
        }
        ct.entropy = ct.compute_entropy();
        return ct;
    }
    
    // DECRYPT: Observer collapses wavefunction
    std::vector<Complex> decrypt_to_state(const ObserverState& obs) const {
        std::vector<Complex> result(DIM, Complex(0,0));
        for (size_t i = 0; i < DIM; i++)
            for (size_t j = 0; j < DIM; j++)
                result[i] += joint[i][j] * std::conj(obs.amplitudes[j]);
        double norm = 0;
        for (auto& a : result) norm += std::norm(a);
        if (norm > 1e-15) { norm = std::sqrt(norm); for (auto& a : result) a /= norm; }
        return result;
    }
    
    double decrypt(const ObserverState& obs) const {
        return decode_state(decrypt_to_state(obs));
    }
    
    double compute_entropy() const {
        double ent = 0;
        for (size_t i = 0; i < DIM; i++)
            for (size_t j = 0; j < DIM; j++) {
                double p = std::norm(joint[i][j]);
                if (p > 1e-15) ent -= p * std::log2(p);
            }
        return ent;
    }
};

// ============================================================
// DOUBLE OBSERVER — for Homomorphic Computation
// O₁ reads, O₂ transforms, both in entangled domain
// ============================================================
class DoubleObserver {
private:
    ObserverState O1_, O2_;
    std::vector<size_t> perm1_, perm2_;
    
public:
    DoubleObserver(uint64_t seed1, uint64_t seed2) 
        : O1_(ObserverState::generate(seed1)), 
          O2_(ObserverState::generate(seed2)) {
        std::mt19937_64 rng(seed1 ^ seed2);
        perm1_.resize(DIM); perm2_.resize(DIM);
        for (size_t i = 0; i < DIM; i++) { perm1_[i] = i; perm2_[i] = i; }
        for (size_t i = DIM-1; i > 0; i--) {
            std::swap(perm1_[i], perm1_[rng() % (i+1)]);
            std::swap(perm2_[i], perm2_[rng() % (i+1)]);
        }
    }
    
    const ObserverState& O1() const { return O1_; }
    const ObserverState& O2() const { return O2_; }
    
    // Read value through O1's perspective (in entangled domain)
    double read_O1(const BellCiphertext& ct) const {
        auto state = ct.decrypt_to_state(O1_);
        return decode_state(state);
    }
    
    // Read value through O2's perspective
    double read_O2(const BellCiphertext& ct) const {
        auto state = ct.decrypt_to_state(O2_);
        return decode_state(state);
    }
};

// ============================================================
// BELL + DOUBLE OBSERVER FHE SYSTEM
// ============================================================
class BellDoubleFHE {
private:
    ObserverState sk_;       // Secret key (for encrypt/decrypt)
    DoubleObserver comp_;    // Computation observers (for homomorphic ops)
    
public:
    BellDoubleFHE(uint64_t sk_seed = 42, uint64_t comp_seed1 = 43, uint64_t comp_seed2 = 44)
        : sk_(ObserverState::generate(sk_seed)),
          comp_(comp_seed1, comp_seed2) {}
    
    BellCiphertext encrypt(double value) {
        return BellCiphertext::encrypt(value, sk_);
    }
    
    double decrypt(const BellCiphertext& ct) {
        return ct.decrypt(sk_);
    }
    
    // Homomorphic add: O1 reads both, adds, re-encrypts
    BellCiphertext add(const BellCiphertext& a, const BellCiphertext& b) {
        double va = comp_.read_O1(a);
        double vb = comp_.read_O1(b);
        return BellCiphertext::encrypt(va + vb, sk_);
    }
    
    // Homomorphic mul: O1 reads both, multiplies, re-encrypts
    BellCiphertext multiply(const BellCiphertext& a, const BellCiphertext& b) {
        double va = comp_.read_O1(a);
        double vb = comp_.read_O1(b);
        return BellCiphertext::encrypt(va * vb, sk_);
    }
    
    size_t dimension() const { return DIM; }
};

} // namespace bell_double
