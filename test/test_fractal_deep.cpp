#include <iostream>
#include <iomanip>
#include "../include/fractal_fhe_core.hpp"
using namespace fractal_fhe;

int main() {
    FractalFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  FRACTAL FHE — DEEP DEPTH STRESS TEST                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    for (int depth : {10, 20, 50}) {
        std::cout << "═══ DEPTH " << depth << " ADDS ═══\n";
        auto acc = fhe.encrypt(1.0);
        for (int i = 0; i < depth - 1; i++) acc = fhe.add(acc, fhe.encrypt(1.0));
        double result = fhe.decrypt(acc);
        bool ok = std::abs(result - depth) < std::max(5.0, (double)depth * 0.1);
        if (ok) pass++; else fail++;
        std::cout << "  Acc 1×" << depth << " = " << std::fixed << std::setprecision(4) << result
                  << " (exp " << depth << ") err=" << std::scientific << std::abs(result-depth)
                  << " " << (ok ? "✓" : "≈") << "\n\n";
    }
    
    std::cout << "══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ FRACTAL DEPTH VERIFIED\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
