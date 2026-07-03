#include <iostream>
#include "../pqcrypto/full/ml_dsa_phi_full.hpp"
using namespace ml_dsa_phi_full;

int main() {
    uint8_t seed[32] = {42};
    auto kp = KeyPair::generate(seed, 32);
    
    // Sign's A
    std::array<uint8_t, CT_BYTES> pk_hash;
    SHAKE256::hash(pk_hash, (const uint8_t*)kp.public_key.coeffs.data(), N * sizeof(int32_t));
    PolyPhi A_sign;
    for (int i = 0; i < N; i++) {
        A_sign.coeffs[i] = (int32_t)(((int64_t)pk_hash[i % CT_BYTES] * 1234567 * (i + 1)) % Q);
    }
    
    // Verify's A (same derivation)
    std::array<uint8_t, CT_BYTES> pk_hash2;
    SHAKE256::hash(pk_hash2, (const uint8_t*)kp.public_key.coeffs.data(), N * sizeof(int32_t));
    PolyPhi A_verify;
    for (int i = 0; i < N; i++) {
        A_verify.coeffs[i] = (int32_t)(((int64_t)pk_hash2[i % CT_BYTES] * 1234567 * (i + 1)) % Q);
    }
    
    bool same = true;
    for (int i = 0; i < 5; i++) {
        if (A_sign.coeffs[i] != A_verify.coeffs[i]) same = false;
        std::cout << i << ": sign=" << A_sign.coeffs[i] << " verify=" << A_verify.coeffs[i] << "\n";
    }
    std::cout << "A matrix same: " << (same ? "YES" : "NO") << "\n";
    
    // Now check: public_key == A * secret_key?
    auto computed_pk = PolyPhi::multiply_exact(A_sign, kp.secret_key);
    bool pk_match = true;
    for (int i = 0; i < 5; i++) {
        if (computed_pk.coeffs[i] != kp.public_key.coeffs[i]) pk_match = false;
        std::cout << i << ": computed_pk=" << computed_pk.coeffs[i] << " stored_pk=" << kp.public_key.coeffs[i] << "\n";
    }
    std::cout << "Public key consistent: " << (pk_match ? "YES" : "NO") << "\n";
    
    return 0;
}
