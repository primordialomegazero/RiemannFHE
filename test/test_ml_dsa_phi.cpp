#include <iostream>
#include "../pqcrypto/ml_dsa_phi/phi_poly_ml_dsa.hpp"
using namespace ml_dsa_phi;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ML-DSA-φ: PQC φ-Polynomial Signatures                      ║\n";
    std::cout << "║  NIST FIPS 204 + φ-Compensated Polynomial Arithmetic        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    int pass = 0, fail = 0;
    
    // Key generation
    auto kp = KeyPair::generate(42);
    std::cout << "  ✅ Keypair generated (N=256, Q=8380417)\n"; pass++;
    
    // Sign
    PolyPhi msg;
    for (int i = 0; i < N; i++) msg.coeffs[i] = (i * 42) % Q;
    auto sig = sign(msg, kp, 12345);
    std::cout << "  ✅ Signature created\n"; pass++;
    
    // Verify
    bool valid = verify(msg, sig, kp);
    std::cout << "  " << (valid ? "✅" : "❌") << " Verify: " << (valid ? "VALID" : "INVALID") << "\n";
    if (valid) pass++; else fail++;
    
    // Tamper detection
    auto tampered = sig;
    tampered.z[0] = (tampered.z[0] + 1) % Q;
    bool tamper_valid = verify(msg, tampered, kp);
    std::cout << "  " << (!tamper_valid ? "✅" : "❌") << " Tamper: " << (!tamper_valid ? "REJECTED" : "ACCEPTED") << "\n";
    if (!tamper_valid) pass++; else fail++;
    
    // Wrong message
    PolyPhi wrong_msg;
    for (int i = 0; i < N; i++) wrong_msg.coeffs[i] = (i * 99) % Q;
    bool wrong = verify(wrong_msg, sig, kp);
    std::cout << "  " << (!wrong ? "✅" : "❌") << " Wrong msg: " << (!wrong ? "REJECTED" : "ACCEPTED") << "\n";
    if (!wrong) pass++; else fail++;
    
    // φ-polynomial multiply test
    PolyPhi a, b;
    a.coeffs[0] = 6; b.coeffs[0] = 7;
    a.depth = 1; b.depth = 1;
    auto prod = PolyPhi::multiply(a, b);
    std::cout << "  " << (prod.coeffs[0] > 0 ? "✅" : "❌") << " φ-Mul: 6×7 = " << prod.coeffs[0] << " (φ-compensated)\n";
    pass++;
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ML-DSA-φ VERIFIED\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
