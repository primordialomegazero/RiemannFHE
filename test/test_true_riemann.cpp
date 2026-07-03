#include <iostream>
#include <iomanip>
#include "../include/true_riemann_fhe.hpp"
using namespace true_riemann;

int main() {
    TrueRiemannFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TRUE RIEMANN FHE — Zeta Zero Attraction                    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    // Encrypt/Decrypt
    std::cout << "═══ ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618, 3.1416, 21, 42, 100, 500, 1000, -7.5, -100};
    for (double t : tests) {
        auto ct = fhe.encrypt(t, 10);
        double dec = fhe.decrypt(ct);
        bool ok = std::abs(t - dec) < 0.01;
        if (ok) pass++; else fail++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(8) << t
                  << " → " << std::setw(12) << std::setprecision(6) << dec
                  << " | " << (ok ? "✓" : "≈") << "\n";
    }
    
    // Homomorphic Add
    std::cout << "\n═══ HOMOMORPHIC ADD ═══\n\n";
    struct { double a, b; const char* l; } adds[] = {
        {15, 25, "15+25"}, {8, 13, "8+13"}, {100, 200, "100+200"},
        {-10, 30, "-10+30"}, {500, 500, "500+500"}, {1000, 500, "1000+500"}
    };
    for (auto [a,b,l] : adds) {
        auto ca = fhe.encrypt(a, 5), cb = fhe.encrypt(b, 10);
        auto csum = fhe.add(ca, cb);
        double r = fhe.decrypt(csum);
        bool ok = std::abs(r - (a+b)) < std::max(1.0, std::abs(a+b)*0.01);
        if (ok) pass++; else fail++;
        std::cout << "  " << l << " = " << r << " (exp " << (a+b) << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    // Homomorphic Mul
    std::cout << "\n═══ HOMOMORPHIC MUL ═══\n\n";
    struct { double a, b; const char* l; } muls[] = {
        {6, 7, "6×7"}, {5, 8, "5×8"}, {10, 10, "10×10"}, {-3, 7, "-3×7"}
    };
    for (auto [a,b,l] : muls) {
        auto ca = fhe.encrypt(a, 3), cb = fhe.encrypt(b, 7);
        auto cp = fhe.multiply(ca, cb);
        double r = fhe.decrypt(cp);
        bool ok = std::abs(r - (a*b)) < std::max(1.0, std::abs(a*b)*0.01);
        if (ok) pass++; else fail++;
        std::cout << "  " << l << " = " << r << " (exp " << (a*b) << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ALL PASS\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
