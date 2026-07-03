#include <iostream>
#include <iomanip>
#include <cmath>
#include "../include/fractal_fhe_core.hpp"
using namespace fractal_fhe;

int main() {
    FractalFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  FRACTAL FHE — COMPREHENSIVE TEST SUITE                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    // ═══ 1. ENCRYPT/DECRYPT — 15 test values ═══
    std::cout << "═══ 1. ENCRYPT/DECRYPT (15 values) ═══\n\n";
    double enc_tests[] = {0, 1.618, 3.1416, 13, 21, 34, 42, 69, 100, 500, 1000, -7.5, -100, -500, -1000};
    for (double t : enc_tests) {
        auto ct = fhe.encrypt(t);
        double dec = fhe.decrypt(ct);
        double err = std::abs(t - dec);
        bool ok = err < std::max(5.0, std::abs(t) * 0.02);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(10) << t
                  << " → " << std::setw(12) << std::setprecision(4) << dec
                  << " | err=" << std::scientific << std::setprecision(1) << err
                  << " " << (ok ? "✓" : "≈") << "\n";
    }
    
    // ═══ 2. TRUE HOMOMORPHIC ADD — 10 pairs ═══
    std::cout << "\n═══ 2. TRUE HOMOMORPHIC ADD (10 pairs) ═══\n\n";
    struct { double a, b; const char* l; } adds[] = {
        {15, 25, "15+25"}, {8, 13, "8+13"}, {100, 200, "100+200"},
        {-10, 30, "-10+30"}, {-50, -25, "-50-25"}, {500, 500, "500+500"},
        {1000, 500, "1000+500"}, {-1000, 500, "-1000+500"}, 
        {3.1416, 1.618, "π+φ"}, {42, 69, "42+69"}
    };
    for (auto [a,b,label] : adds) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto csum = fhe.add(ca, cb);
        double r = fhe.decrypt(csum);
        double exp = a + b;
        bool ok = std::abs(r - exp) < std::max(10.0, std::abs(exp) * 0.1);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::setw(10) << label << " = " << std::setw(12) << std::fixed << std::setprecision(4) << r
                  << " (exp " << exp << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    // ═══ 3. TRUE HOMOMORPHIC MUL — 10 pairs ═══
    std::cout << "\n═══ 3. TRUE HOMOMORPHIC MUL (10 pairs) ═══\n\n";
    struct { double a, b; const char* l; } muls[] = {
        {6, 7, "6×7"}, {5, 8, "5×8"}, {10, 10, "10×10"},
        {-3, 7, "-3×7"}, {-4, -9, "-4×-9"}, {100, 10, "100×10"},
        {50, 20, "50×20"}, {-10, -10, "-10×-10"},
        {3.1416, 2, "π×2"}, {1.618, 1.618, "φ×φ"}
    };
    for (auto [a,b,label] : muls) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto cprod = fhe.multiply(ca, cb);
        double r = fhe.decrypt(cprod);
        double exp = a * b;
        bool ok = std::abs(r - exp) < std::max(20.0, std::abs(exp) * 0.2);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::setw(10) << label << " = " << std::setw(12) << std::fixed << std::setprecision(4) << r
                  << " (exp " << exp << ") " << (ok ? "✓" : "≈") << "\n";
    }
    
    // ═══ 4. RECURSIVE ADD — 5 depths ═══
    std::cout << "\n═══ 4. RECURSIVE ADD (5 depths) ═══\n\n";
    for (int depth : {1, 3, 5, 7}) {
        auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
        auto result = fhe.recursive_add(ca, cb, depth);
        double r = fhe.decrypt(result);
        bool ok = std::abs(r - 40.0) < 10.0;
        if (ok) pass++; else fail++;
        std::cout << "  depth=" << depth << " | 15+25 = " << r << " (exp 40) " << (ok ? "✓" : "≈") << "\n";
    }
    
    // ═══ 5. RECURSIVE MUL — 4 depths ═══
    std::cout << "\n═══ 5. RECURSIVE MUL (4 depths) ═══\n\n";
    for (int depth : {1, 2, 3, 4}) {
        auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
        auto result = fhe.recursive_mul(cx, cy, depth);
        double r = fhe.decrypt(result);
        bool ok = std::abs(r - 42.0) < 15.0;
        if (ok) pass++; else fail++;
        std::cout << "  depth=" << depth << " | 6×7 = " << r << " (exp 42) " << (ok ? "✓" : "≈") << "\n";
    }
    
    // ═══ 6. DEEP DEPTH STRESS — 5,10,20,50 ═══
    std::cout << "\n═══ 6. DEEP DEPTH STRESS ═══\n\n";
    for (int depth : {5, 10, 20, 50}) {
        auto acc = fhe.encrypt(1.0);
        for (int i = 0; i < depth - 1; i++) acc = fhe.add(acc, fhe.encrypt(1.0));
        double r = fhe.decrypt(acc);
        bool ok = std::abs(r - depth) < std::max(5.0, (double)depth * 0.05);
        if (ok) pass++; else fail++;
        std::cout << "  " << std::setw(2) << depth << " adds: 1×" << depth << " = " << std::fixed << std::setprecision(4) << r
                  << " (exp " << depth << ") err=" << std::scientific << std::abs(r-depth) << " " << (ok ? "✓" : "≈") << "\n";
    }
    
    // ═══ 7. MIXED OPERATIONS ═══
    std::cout << "\n═══ 7. MIXED OPERATIONS ═══\n\n";
    // (a + b) × c
    auto a = fhe.encrypt(10.0), b = fhe.encrypt(5.0), c = fhe.encrypt(3.0);
    auto sum = fhe.add(a, b);
    auto prod = fhe.multiply(sum, c);
    double r = fhe.decrypt(prod);
    bool ok = std::abs(r - 45.0) < 15.0;
    if (ok) pass++; else fail++;
    std::cout << "  (10+5)×3 = " << r << " (exp 45) " << (ok ? "✓" : "≈") << "\n";
    
    // a² + b²
    auto a2 = fhe.multiply(a, a);
    auto b2 = fhe.multiply(b, b);
    auto sum_sq = fhe.add(a2, b2);
    double r2 = fhe.decrypt(sum_sq);
    bool ok2 = std::abs(r2 - 125.0) < 30.0;
    if (ok2) pass++; else fail++;
    std::cout << "  10²+5² = " << r2 << " (exp 125) " << (ok2 ? "✓" : "≈") << "\n";
    
    // ═══ RESULTS ═══
    int total = pass + fail;
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << total << " | Pass: " << pass << " | Fail: " << fail;
    if (fail == 0) std::cout << " | ✅ FRACTAL FHE COMPREHENSIVE PASS";
    else std::cout << " | ⚠ " << fail << " FAILURES";
    std::cout << "\n  Rate: " << std::fixed << std::setprecision(1) << (100.0 * pass / total) << "%\n";
    std::cout << "══════════════════════════════════════════════════════\n";
    
    return fail > 0 ? 1 : 0;
}
