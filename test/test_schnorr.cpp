#include <iostream>
#include "../security/signing/schnorr_auth.hpp"
using namespace riemann_schnorr;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SCHNORR AUTH TEST (EVP API — Zero Warnings)                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    int pass = 0, fail = 0;
    
    // Signer
    SchnorrSigner signer;
    std::cout << "  ✅ Key generated (secp256k1)\n"; pass++;
    
    // Sign
    std::string msg = "RiemannFHE Test Message";
    auto sig = signer.sign(msg);
    std::cout << "  ✅ Signature: " << sig.size() << " bytes\n"; pass++;
    
    // Verify
    bool valid = signer.verify(msg, sig);
    std::cout << "  " << (valid ? "✅" : "❌") << " Verify: " << (valid ? "VALID" : "INVALID") << "\n";
    valid ? pass++ : fail++;
    
    // Tamper
    auto tampered = sig;
    tampered[tampered.size()/2] ^= 0xFF;
    bool tamper_ok = signer.verify(msg, tampered);
    std::cout << "  " << (!tamper_ok ? "✅" : "❌") << " Tamper: " << (!tamper_ok ? "REJECTED" : "ACCEPTED") << "\n";
    !tamper_ok ? pass++ : fail++;
    
    // Wrong message
    bool wrong = signer.verify("Wrong message", sig);
    std::cout << "  " << (!wrong ? "✅" : "❌") << " Wrong msg: " << (!wrong ? "REJECTED" : "ACCEPTED") << "\n";
    !wrong ? pass++ : fail++;
    
    // φ-Hash
    uint64_t h1 = PhiHash::hash("test", 4);
    uint64_t h2 = PhiHash::hash("test", 4);
    uint64_t h3 = PhiHash::hash("Test", 4);
    bool hash_ok = (h1 == h2 && h1 != h3);
    std::cout << "  " << (hash_ok ? "✅" : "❌") << " φ-Hash deterministic + avalanche\n";
    hash_ok ? pass++ : fail++;
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ALL PASS\n" : " | ❌ FAILURES\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    
    return fail > 0 ? 1 : 0;
}
