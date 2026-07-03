#include <iostream>
#include <iomanip>
#include "../include/true_fhe.hpp"
using namespace true_fhe;

int main() {
    TrueFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TRUE FHE — Direct Ciphertext Only — Zero Extraction        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "═══ ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618, 3.1416, 21, 42, 100, 500, 1000, -7.5, -100, -500, -1000};
    for (double t : tests) {
        auto ct = fhe.encrypt(t);
        double dec = fhe.decrypt(ct);
        bool ok = std::abs(t - dec) < 1.0;
        if (ok) pass++; else fail++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(8) << t
                  << " → " << std::setw(12) << std::setprecision(6) << dec
                  << " " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ ADD ═══\n\n";
    struct { double a, b; const char* l; } adds[] = {
        {15, 25, "15+25"}, {8, 13, "8+13"}, {100, 200, "100+200"},
        {500, 500, "500+500"}, {1000, 500, "1000+500"}, {-10, 30, "-10+30"}
    };
    for (auto [a,b,l] : adds) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto csum = fhe.add(ca, cb);
        double r = fhe.decrypt(csum);
        bool ok = std::abs(r - (a+b)) < 5.0;
        if (ok) pass++; else fail++;
        std::cout << "  " << l << " = " << std::fixed << std::setprecision(4) << r
                  << " (exp " << (a+b) << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ MUL ═══\n\n";
    struct { double a, b; const char* l; } muls[] = {
        {6, 7, "6×7"}, {5, 8, "5×8"}, {10, 10, "10×10"},
        {-3, 7, "-3×7"}, {100, 10, "100×10"}
    };
    for (auto [a,b,l] : muls) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto cp = fhe.multiply(ca, cb);
        double r = fhe.decrypt(cp);
        bool ok = std::abs(r - (a*b)) < 10.0;
        if (ok) pass++; else fail++;
        std::cout << "  " << l << " = " << std::fixed << std::setprecision(4) << r
                  << " (exp " << (a*b) << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    std::cout << "\n═══ DEPTH: 5 ADDS ═══\n";
    auto acc = fhe.encrypt(1.0);
    for (int i = 0; i < 4; i++) acc = fhe.add(acc, fhe.encrypt(1.0));
    double dr = fhe.decrypt(acc);
    bool dok = std::abs(dr - 5.0) < 2.0;
    if (dok) pass++; else fail++;
    std::cout << "  Acc 1×5 = " << dr << " (exp 5) " << (dok ? "✓" : "≈") << "\n";
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ALL PASS\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
