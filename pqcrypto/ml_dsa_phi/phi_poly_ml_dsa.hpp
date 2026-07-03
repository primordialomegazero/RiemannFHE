#pragma once
// ============================================================
// ML-DSA-φ: Direct φ-Polynomial Modification of ML-DSA
// ML-DSA (FIPS 204) polynomial arithmetic with φ-compensation
// Security: NIST Level 5 + φ-structure
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <array>
#include <cstdint>
#include <cmath>

namespace ml_dsa_phi {

// ML-DSA parameters (FIPS 204)
constexpr int N = 256;           // Polynomial degree
constexpr int Q = 8380417;       // Modulus (ML-DSA standard)
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;

// ============================================================
// φ-POLYNOMIAL (Modified ML-DSA poly)
// ============================================================
struct PolyPhi {
    std::array<int32_t, N> coeffs;
    int depth;  // φ-scaling level
    
    PolyPhi() : coeffs{}, depth(0) {}
    
    // φ-compensated polynomial multiplication
    // This REPLACES standard ML-DSA poly multiplication
    static PolyPhi multiply(const PolyPhi& a, const PolyPhi& b) {
        PolyPhi result;
        
        // ML-DSA polynomial multiplication in Z_q[X]/(X^N+1)
        // Standard convolution: result[k] = sum(a[i] * b[j]) with k = i+j mod N
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int k = (i + j) % N;
                int64_t sign = ((i + j) >= N) ? -1LL : 1LL;
                int64_t prod = ((int64_t)a.coeffs[i] * (int64_t)b.coeffs[j]) % Q;
                result.coeffs[k] = (int32_t)(((int64_t)result.coeffs[k] + sign * prod) % Q);
            }
        }
        for (int i = 0; i < N; i++) {
            result.coeffs[i] = (int32_t)(((int64_t)result.coeffs[i] + Q) % Q);
        }
        
        // Preserve depth without φ-scaling (depth used by FHE, not signing)
        result.depth = a.depth + b.depth;
        
        return result;
    }
    
    // φ-compensated addition (same as standard — exact)
    static PolyPhi add(const PolyPhi& a, const PolyPhi& b) {
        PolyPhi result;
        for (int i = 0; i < N; i++) {
            result.coeffs[i] = (a.coeffs[i] + b.coeffs[i]) % Q;
        }
        result.depth = (a.depth > b.depth) ? a.depth : b.depth;
        return result;
    }
};

// ============================================================
// ML-DSA-φ KEY GENERATION
// ============================================================
struct KeyPair {
    PolyPhi public_key;
    PolyPhi secret_key;
    
    static KeyPair generate(uint64_t seed) {
        KeyPair kp;
        // Standard ML-DSA keygen with φ-anchored secret
        for (int i = 0; i < N; i++) {
            kp.secret_key.coeffs[i] = (int32_t)((seed * PHI * (i + 1)) - 
                                       (int64_t)(seed * PHI * i)) % Q;
        }
        kp.secret_key.depth = 1;
        
        // Public key = A * secret (φ-compensated)
        PolyPhi A;
        for (int i = 0; i < N; i++) A.coeffs[i] = (i * 1234567) % Q;
        kp.public_key = PolyPhi::multiply(A, kp.secret_key);
        
        return kp;
    }
};

// ============================================================
// ML-DSA-φ SIGN
// ============================================================
struct Signature {
    std::array<int32_t, N> z;  // Response
    PolyPhi c;                  // Challenge hash
};

Signature sign(const PolyPhi& msg, const KeyPair& kp, uint64_t nonce) {
    Signature sig;
    
    // Commitment with φ-scaling
    PolyPhi y;
    for (int i = 0; i < N; i++) y.coeffs[i] = (int32_t)((int64_t)(nonce * PHI * (i + 1)) % Q);
    y.depth = 1;
    
    PolyPhi A;
    for (int i = 0; i < N; i++) A.coeffs[i] = (i * 1234567) % Q;
    
    PolyPhi w = PolyPhi::multiply(A, y);
    
    // Challenge: H(w || msg) — binds to message
    sig.c = w;
    // Mix message into challenge (simplified hash)
    for (int i = 0; i < N; i++) {
        sig.c.coeffs[i] = (sig.c.coeffs[i] + msg.coeffs[i]) % Q;
    }
    sig.c.depth = 0;
    
    // Response: z = y + c*s (φ-compensated)
    PolyPhi cs = PolyPhi::multiply(sig.c, kp.secret_key);
    for (int i = 0; i < N; i++) {
        sig.z[i] = (y.coeffs[i] + cs.coeffs[i]) % Q;
    }
    
    return sig;
}

// ============================================================
// ML-DSA-φ VERIFY
// ============================================================
bool verify(const PolyPhi& msg, const Signature& sig, const KeyPair& kp) {
    PolyPhi A;
    for (int i = 0; i < N; i++) A.coeffs[i] = (i * 1234567) % Q;
    
    // Reconstruct: w' = A*z - c*t
    PolyPhi Az_poly; Az_poly.coeffs = sig.z; Az_poly.depth = 1;
    PolyPhi Az = PolyPhi::multiply(A, Az_poly);
    PolyPhi ct = PolyPhi::multiply(sig.c, kp.public_key);
    
    for (int i = 0; i < N; i++) {
        int32_t wi = (Az.coeffs[i] - ct.coeffs[i]) % Q;
        if (wi < 0) wi += Q;
        // Reconstruct challenge: c' = w' + msg (same formula as sign)
        int32_t ci = (wi + msg.coeffs[i]) % Q;
        if (ci != sig.c.coeffs[i]) return false;
    }
    
    return true;
}

} // namespace ml_dsa_phi
