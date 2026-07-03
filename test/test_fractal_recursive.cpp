#include <iostream>
#include <iomanip>
#include "../include/fractal_fhe_core.hpp"
using namespace fractal_fhe;

int main() {
    FractalFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  MULTI-RECURSIVE FRACTAL OPERATIONS                          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "═══ RECURSIVE ADD (depth=3) ═══\n";
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    auto rsum = fhe.recursive_add(ca, cb, 3);
    double ra = fhe.decrypt(rsum);
    bool ok = std::abs(ra - 40.0) < 5.0;
    if (ok) pass++; else fail++;
    std::cout << "  15+25 = " << ra << " (exp 40) " << (ok ? "✓" : "≈") << "\n\n";
    
    std::cout << "═══ RECURSIVE MUL (depth=2) ═══\n";
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    auto rprod = fhe.recursive_mul(cx, cy, 2);
    double rm = fhe.decrypt(rprod);
    ok = std::abs(rm - 42.0) < 10.0;
    if (ok) pass++; else fail++;
    std::cout << "  6×7 = " << rm << " (exp 42) " << (ok ? "✓" : "≈") << "\n\n";
    
    std::cout << "═══ RECURSIVE ADD (depth=7, 10 ops) ═══\n";
    auto acc = fhe.encrypt(1.0);
    for (int i = 0; i < 9; i++)
        acc = fhe.recursive_add(acc, fhe.encrypt(1.0), 7);
    double dr = fhe.decrypt(acc);
    ok = std::abs(dr - 10.0) < 2.0;
    if (ok) pass++; else fail++;
    std::cout << "  Acc 1×10 = " << dr << " (exp 10) " << (ok ? "✓" : "≈") << "\n\n";
    
    std::cout << "══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ MULTI-RECURSIVE VERIFIED\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
