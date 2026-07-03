#include <iostream>
#include <iomanip>
#include <cmath>
#include "../include/true_fhe.hpp"
using namespace true_fhe;

int main() {
    TrueFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  FRACTAL TRUE FHE — DEEP DEPTH STRESS TEST                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    for (int depth : {10, 20, 50, 100}) {
        std::cout << "═══ DEPTH " << depth << " ADDS ═══\n";
        auto acc = fhe.encrypt(1.0);
        for (int i = 0; i < depth - 1; i++) {
            acc = fhe.add(acc, fhe.encrypt(1.0));
        }
        double result = fhe.decrypt(acc);
        double error = std::abs(result - depth);
        bool ok = error < std::max(1.0, (double)depth * 0.01);
        if (ok) pass++; else fail++;
        std::cout << "  Acc 1×" << std::setw(3) << depth << " = " << std::fixed << std::setprecision(4) << result
                  << " (exp " << depth << ") err=" << std::scientific << error << " " << (ok ? "✓" : "≈") << "\n\n";
    }
    
    std::cout << "\n═══ DEEP MUL CHAIN ═══\n";
    // 2^10 = 1024 via 10 consecutive multiplies
    auto val = fhe.encrypt(2.0);
    for (int i = 0; i < 10; i++) {
        val = fhe.multiply(val, fhe.encrypt(2.0));
    }
    double mul_result = fhe.decrypt(val);
    double expected = std::pow(2.0, 11);  // 2^11 = 2048
    bool mok = std::abs(mul_result - expected) < expected * 0.1;
    if (mok) pass++; else fail++;
    std::cout << "  2^11 = " << std::fixed << std::setprecision(4) << mul_result
              << " (exp " << expected << ") " << (mok ? "✓" : "≈") << "\n\n";
    
    std::cout << "══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ DEEP DEPTH VERIFIED\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
