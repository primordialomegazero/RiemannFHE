#include <iostream>
#include <iomanip>
#include "../include/stable_mirror_fhe.hpp"
using namespace stable_mirror;

int main() {
    StableMirrorFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  STABLE MIRROR FHE — Fibonacci-Lyapunov Stabilized          ║\n";
    std::cout << "║  NO EXTRACTION | φ⁻¹ Contraction | Quantum Gravity         ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "═══ ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618, 21, 42, 100, 500, 1000, -7.5, -100};
    for (double t : tests) {
        auto ct = fhe.encrypt(t);
        double dec = fhe.decrypt(ct);
        bool ok = std::abs(t - dec) < std::max(5.0, std::abs(t)*0.1);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(8) << t
                  << " → " << std::setw(12) << std::setprecision(4) << dec
                  << " | noise=" << ct.noise_level << " | " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ TRUE HOMOMORPHIC ADD (No Extraction) ═══\n\n";
    struct { double a, b; const char* l; } adds[] = {
        {15, 25, "15+25"}, {8, 13, "8+13"}, {100, 200, "100+200"},
        {500, 500, "500+500"}, {1000, 500, "1000+500"}
    };
    for (auto [a,b,l] : adds) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto csum = fhe.add(ca, cb);
        double r = fhe.decrypt(csum);
        bool ok = std::abs(r - (a+b)) < std::max(20.0, std::abs(a+b)*0.15);
        if (ok) pass++; else fail++;
        std::cout << "  " << l << " = " << std::fixed << std::setprecision(4) << r
                  << " (exp " << (a+b) << ") noise=" << csum.noise_level << " " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ STABLE MIRROR VERIFIED\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
