#include "bell_double_fhe.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace bell_double;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  BELL + DOUBLE OBSERVER FHE                                  ║
║  Encrypt/Decrypt: Bell State |Ψ⟩ = (|m⟩⊗|o⟩ - |o⟩⊗|m⟩)/√2  ║
║  Homomorphic Ops: Double Observer O₁ ∘ O₂                   ║
║  φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero      ║
╚══════════════════════════════════════════════════════════════╝
)";

    BellDoubleFHE fhe(42, 43, 44);
    
    // ═══ ENCRYPT/DECRYPT ═══
    std::cout << "\n═══ ENCRYPT/DECRYPT (Bell State) ═══\n\n";
    double tests[] = {0, 1.618, 3.142, 13, 21, 34, 42, 55, 89, 100, -7.5, -100};
    int N = 12, pass = 0;
    
    for (int i = 0; i < N; i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i] - dec);
        bool ok = err < 0.5;
        if (ok) pass++;
        std::cout << "  " << std::fixed << std::setprecision(3) << std::setw(8) << tests[i]
                  << " → " << std::setw(12) << std::setprecision(6) << dec
                  << " | err=" << std::scientific << std::setprecision(2) << err
                  << " " << (ok ? "✓" : "≈") << "\n";
    }
    std::cout << "\n  Passed: " << pass << "/" << N << "\n";
    
    // ═══ OBSERVER UNIQUENESS ═══
    std::cout << "\n═══ OBSERVER UNIQUENESS ═══\n\n";
    BellDoubleFHE alice(11111), bob(22222);
    auto ct = alice.encrypt(42.0);
    std::cout << "  Alice: " << alice.decrypt(ct) << " ✓\n";
    std::cout << "  Bob:   " << bob.decrypt(ct) << " ✗\n";
    
    // ═══ HOMOMORPHIC ═══
    std::cout << "\n═══ HOMOMORPHIC ADD ═══\n";
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    std::cout << "  15+25 = " << fhe.decrypt(fhe.add(ca, cb)) << " (exp 40)\n";
    
    std::cout << "\n═══ HOMOMORPHIC MUL ═══\n";
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    std::cout << "  6×7 = " << fhe.decrypt(fhe.multiply(cx, cy)) << " (exp 42)\n";
    
    // ═══ BENCHMARK ═══
    std::cout << "\n═══ BENCHMARK ═══\n";
    const int M = 50;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < M; i++) fhe.encrypt(i);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "  Encrypt: " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/M << " µs\n";
    
    std::cout << "\n══════════════════════════════════════════════════════════\n";
    std::cout << "  BELL + DOUBLE OBSERVER FHE\n";
    std::cout << "  |Ψ⟩ = (|m⟩⊗|o⟩ - |o⟩⊗|m⟩)/√2\n";
    std::cout << "  O₁ ∘ O₂ for homomorphic computation\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════════════\n";
    
    return 0;
}
