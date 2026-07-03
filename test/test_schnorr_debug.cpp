#include <iostream>
#include "../security/signing/schnorr_auth.hpp"
using namespace riemann_schnorr;

int main() {
    SchnorrSigner s1, s2;
    std::string msg = "test";
    
    // Sign with s1
    auto sig = s1.sign(msg);
    std::cout << "Sig size: " << sig.size() << "\n";
    
    // Verify with s1 (same key)
    bool v1 = s1.verify(msg, sig);
    std::cout << "Self-verify: " << (v1 ? "PASS" : "FAIL") << "\n";
    
    // Verify with s2 (different key)
    bool v2 = s2.verify(msg, sig);
    std::cout << "Cross-verify: " << (v2 ? "PASS" : "FAIL") << " (should FAIL)\n";
    
    // Sign and verify with s2
    auto sig2 = s2.sign(msg);
    bool v3 = s2.verify(msg, sig2);
    std::cout << "Self-verify s2: " << (v3 ? "PASS" : "FAIL") << "\n";
    
    return !v1 || v2 || !v3;
}
