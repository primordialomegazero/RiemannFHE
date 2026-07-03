#include <iostream>
#include "../pqcrypto/full/ml_dsa_phi_full.hpp"
using namespace ml_dsa_phi_full;

int main() {
    uint8_t seed[32] = {42};
    auto kp = KeyPair::generate(seed, 32);
    
    PolyPhi msg;
    msg.coeffs[0] = 42000;
    
    uint8_t nonce[32] = {99};
    auto sig = sign(msg, kp, nonce, 32);
    
    // Manual verify — same A as sign
    std::array<uint8_t, CT_BYTES> rho;
    SHAKE256::hash(rho, nonce, 32);
    PolyPhi A;
    for (int i = 0; i < N; i++) {
        A.coeffs[i] = (int32_t)(((int64_t)rho[i % CT_BYTES] * 1234567 * (i + 1)) % Q);
    }
    
    PolyPhi Az = PolyPhi::multiply_ntt(A, sig.z);
    PolyPhi ct = PolyPhi::multiply_ntt(sig.c, kp.public_key);
    PolyPhi w_prime = PolyPhi::sub(Az, ct);
    
    uint8_t challenge[64] = {};
    for (int i = 0; i < 32 && i < N; i++) {
        challenge[i] = (uint8_t)(w_prime.coeffs[i] & 0xFF);
        challenge[i+32] = (uint8_t)(msg.coeffs[i] & 0xFF);
    }
    std::array<uint8_t, CT_BYTES> c_hash;
    SHAKE256::hash(c_hash, challenge, 64);
    
    PolyPhi c_prime;
    c_prime.coeffs.fill(0);
    for (int i = 0; i < CT_BYTES; i++) c_prime.coeffs[i] = c_hash[i] % Q;
    
    bool match = CT::equal(sig.c, c_prime);
    std::cout << "Manual verify: " << (match ? "VALID" : "INVALID") << "\n";
    
    // Check first 3 coeffs
    for (int i = 0; i < 3; i++) {
        std::cout << i << ": sig.c=" << sig.c.coeffs[i] << " c_prime=" << c_prime.coeffs[i] 
                  << " w'=" << w_prime.coeffs[i] << " Az=" << Az.coeffs[i] << " ct=" << ct.coeffs[i] << "\n";
    }
    
    return 0;
}
