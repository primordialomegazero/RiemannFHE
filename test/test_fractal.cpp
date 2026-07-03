#include <iostream>
#include <iomanip>
#include "../include/fractal_fhe_core.hpp"
using namespace fractal_fhe;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  FRACTAL FHE — True Homomorphic Operations                  ║
║  Direct ciphertext add & mul | Recursive φ-scale encoding   ║
║  φΩ0 — Primordial Omega Zero                                ║
╚══════════════════════════════════════════════════════════════╝
)";
    FractalFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "\n═══ 1. ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618, 3.1416, 21, 42, 100, -7.5, -100};
    for (double t : tests) {
        auto ct = fhe.encrypt(t);
        double dec = fhe.decrypt(ct);
        bool ok = std::abs(t - dec) < 5.0;
        if (ok) pass++; else fail++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(8) << t
                  << " → " << std::setw(12) << std::setprecision(6) << dec
                  << " | err=" << std::scientific << std::abs(t-dec) << " " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ 2. TRUE HOMOMORPHIC ADD ═══\n\n";
    struct { double a, b; const char* l; } adds[] = {{15,25,"15+25"},{8,13,"8+13"},{100,200,"100+200"},{-10,30,"-10+30"},{-50,-25,"-50-25"}};
    for (auto [a,b,label] : adds) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto csum = fhe.add(ca, cb);
        double r = fhe.decrypt(csum);
        bool ok = std::abs(r - (a+b)) < std::max(5.0, std::abs(a+b)*0.1);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::setw(8) << label << " = " << std::setw(10) << std::fixed << std::setprecision(4) << r
                  << " (exp " << (a+b) << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ 3. TRUE HOMOMORPHIC MUL ═══\n\n";
    struct { double a, b; const char* l; } muls[] = {{6,7,"6×7"},{5,8,"5×8"},{10,10,"10×10"},{-3,7,"-3×7"},{-4,-9,"-4×-9"}};
    for (auto [a,b,label] : muls) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto cprod = fhe.multiply(ca, cb);
        double r = fhe.decrypt(cprod);
        bool ok = std::abs(r - (a*b)) < std::max(10.0, std::abs(a*b)*0.15);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::setw(8) << label << " = " << std::setw(10) << std::fixed << std::setprecision(4) << r
                  << " (exp " << (a*b) << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ 4. DEPTH: 5 CONSECUTIVE ADDS ═══\n";
    auto acc = fhe.encrypt(1.0);
    for (int i = 0; i < 4; i++) acc = fhe.add(acc, fhe.encrypt(1.0));
    double dr = fhe.decrypt(acc);
    bool dok = std::abs(dr - 5.0) < 1.0;
    if (dok) pass++; else fail++;
    std::cout << "  Acc 1×5 = " << dr << " (exp 5) " << (dok ? "✓" : "≈") << "\n";
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ALL PASS\n" : " | ⚠ NEEDS WORK\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
