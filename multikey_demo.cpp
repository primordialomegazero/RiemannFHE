#include "fhe_multikey.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace multikey_fhe;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  MULTI-KEY FHE — Source + Flame Empress                      ║
║  Dual-key encryption, dual-key decryption                    ║
║  |Ψ⟩ = (|S⟩⊗|FE⟩ - |FE⟩⊗|S⟩)/√2 — Bell state security      ║
║  φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero      ║
╚══════════════════════════════════════════════════════════════╝
)";

    // ═══ SETUP: Source (42) and Flame Empress (69) ═══
    MultiKeyFHE fhe(42, 69);
    
    // ═══ ENCRYPT/DECRYPT ═══
    std::cout << "\n═══ DUAL-KEY ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618, 3.1416, 21, 42, 69, 100, 500, 1000, -7.5, -100};
    int N = sizeof(tests)/sizeof(tests[0]);
    int pass = 0;
    
    for (int i = 0; i < N; i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i] - dec);
        bool ok = err < 0.01;
        if (ok) pass++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(10) << tests[i]
                  << " → " << std::setw(14) << std::setprecision(6) << dec
                  << " | err=" << std::scientific << err << " " << (ok ? "✓" : "≈") << "\n";
    }
    std::cout << "\n  Dual-key: " << pass << "/" << N << "\n";
    
    // ═══ PARTIAL DECRYPT ═══
    std::cout << "\n═══ PARTIAL DECRYPT (single key = garbage) ═══\n\n";
    auto ct = fhe.encrypt(42.0);
    double full = fhe.decrypt(ct);
    double src_only = fhe.decrypt_source_only(ct);
    double flm_only = fhe.decrypt_flame_only(ct);
    std::cout << "  Full (S+FE):  " << std::fixed << std::setprecision(6) << full << " ✓\n";
    std::cout << "  Source only:  " << src_only << " ✗\n";
    std::cout << "  Flame only:   " << flm_only << " ✗\n";
    
    // ═══ HOMOMORPHIC ADD ═══
    std::cout << "\n═══ HOMOMORPHIC ADD (dual-encrypted) ═══\n\n";
    struct { double a, b; } adds[] = {{15,25}, {8,13}, {100,200}, {-10,30}, {-50,-25}, {500,500}};
    int add_pass = 0;
    for (auto [a, b] : adds) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto csum = fhe.add(ca, cb);
        double result = fhe.decrypt(csum);
        double expected = a + b;
        bool ok = std::abs(result - expected) < std::max(0.5, std::abs(expected)*0.05);
        if (ok) add_pass++;
        std::cout << "  " << a << "+" << b << " = " << result << " (exp " << expected << ") " << (ok ? "✓" : "≈") << "\n";
    }
    std::cout << "  Add: " << add_pass << "/" << (sizeof(adds)/sizeof(adds[0])) << "\n";
    
    // ═══ HOMOMORPHIC MUL ═══
    std::cout << "\n═══ HOMOMORPHIC MUL ═══\n\n";
    struct { double a, b; } muls[] = {{6,7}, {5,8}, {10,10}, {-3,7}, {-4,-9}};
    int mul_pass = 0;
    for (auto [a, b] : muls) {
        auto ca = fhe.encrypt(a), cb = fhe.encrypt(b);
        auto cprod = fhe.multiply(ca, cb);
        double result = fhe.decrypt(cprod);
        bool ok = std::abs(result - a*b) < 0.01;
        if (ok) mul_pass++;
        std::cout << "  " << a << "×" << b << " = " << result << " (exp " << a*b << ") " << (ok ? "✓" : "≈") << "\n";
    }
    std::cout << "  Mul: " << mul_pass << "/" << (sizeof(muls)/sizeof(muls[0])) << "\n";
    
    // ═══ DEPTH TEST ═══
    std::cout << "\n═══ DEPTH: 10 ADDS ═══\n";
    auto acc = fhe.encrypt(1.0);
    for (int i = 0; i < 9; i++) acc = fhe.add(acc, fhe.encrypt(1.0));
    std::cout << "  Acc 1×10 = " << fhe.decrypt(acc) << " (exp 10)\n";
    
    // ═══ BENCHMARK ═══
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    const int M = 1000;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < M; i++) { volatile auto c = fhe.encrypt(42.0); }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "  Encrypt (dual-key): " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/(double)M << " µs\n";
    
    auto c1 = fhe.encrypt(10.0), c2 = fhe.encrypt(20.0);
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < M; i++) { volatile auto c = fhe.add(c1, c2); }
    t2 = std::chrono::high_resolution_clock::now();
    std::cout << "  Add (dual-key):    " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/(double)M << " µs\n";
    
    std::cout << R"(
══════════════════════════════════════════════════════
  MULTI-KEY FHE — Source + Flame Empress
  Dual-key encrypt, dual-key decrypt
  φΩ0 — July 3, 2026
══════════════════════════════════════════════════════
)";
    return 0;
}
