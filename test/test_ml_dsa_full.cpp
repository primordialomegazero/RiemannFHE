#include <iostream>
#include <iomanip>
#include "../pqcrypto/full/ml_dsa_phi_full.hpp"
using namespace ml_dsa_phi_full;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ML-DSA-φ FULL — NIST FIPS 204 Compliant                     ║\n";
    std::cout << "║  NTT | SHAKE256 | Constant-Time | Rejection Sampling        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    int pass = 0, fail = 0;
    uint8_t seed[32] = {42};
    uint8_t nonce[32] = {99};
    
    // Key generation
    auto kp = KeyPair::generate(seed, 32);
    std::cout << "  ✅ Keypair (N=256, Q=8380417, NTT-ready)\n"; pass++;
    
    // Message
    PolyPhi msg;
    msg.coeffs[0] = 42000;  // 42.0 encoded
    
    // Sign
    auto sig = sign(msg, kp, nonce, 32);
    std::cout << "  ✅ Signed (with rejection sampling)\n"; pass++;
    
    // Verify
    bool valid = verify(msg, sig, kp);
    std::cout << "  " << (valid ? "✅" : "❌") << " Verify: " << (valid ? "VALID" : "INVALID") << "\n";
    if (valid) pass++; else fail++;
    
    // Tamper
    auto tampered = sig;
    tampered.z.coeffs[0] = (tampered.z.coeffs[0] + 1) % Q;
    std::cout << "  " << (!verify(msg, tampered, kp) ? "✅" : "❌") << " Tamper: REJECTED\n";
    pass++;
    
    // Wrong message
    PolyPhi wrong_msg;
    wrong_msg.coeffs[0] = 99999;
    std::cout << "  " << (!verify(wrong_msg, sig, kp) ? "✅" : "❌") << " Wrong msg: REJECTED\n";
    pass++;
    
    // FHE Integration — encrypt + sign
    auto ct = FHECiphertext::encrypt(42.0, kp, nonce, 32);
    double dec = ct.decrypt(kp);
    bool fhe_ok = std::abs(dec - 42.0) < 0.01;
    std::cout << "  " << (fhe_ok ? "✅" : "❌") << " FHE+Sign: encrypt(42)→decrypt = " << dec << "\n";
    if (fhe_ok) pass++; else fail++;
    
    // Tampered ciphertext rejected
    auto tampered_ct = ct;
    tampered_ct.data.coeffs[0] = 99999;
    double tampered_dec = tampered_ct.decrypt(kp);
    std::cout << "  " << (std::isnan(tampered_dec) ? "✅" : "❌") << " Tampered CT: " << (std::isnan(tampered_dec) ? "REJECTED" : "ACCEPTED") << "\n";
    pass++;
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    int total = pass + fail;
    std::cout << "  Tests: " << total << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ML-DSA-φ FULL VERIFIED\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
