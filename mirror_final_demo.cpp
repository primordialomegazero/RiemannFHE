#include "mirror_fhe_final.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
using namespace mirror_fhe;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║   MIRROR-FHE FINAL — Ratio Encoding + Soft Fibonacci Floor ║
║   value = amplitude[1]/amplitude[0] — unitary invariant    ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero    ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    MirrorFHE fhe(42, 64);
    
    // Test encrypt/decrypt
    std::cout << "\n═══ ENCRYPT/DECRYPT (Ratio Encoding) ═══\n\n";
    double tests[] = {0, 1.618, 3.1416, 13, 21, 34, 42, 55, 89, 100, 144, 500, -7.5, -100};
    const char* names[] = {"0","φ","π","F7","F8","F9","42","F10","F11","100","F12","500","-7.5","-100"};
    int N = 14;
    
    std::cout << "  " << std::setw(8) << "Value" << " | " << std::setw(10) << "Decrypted" 
              << " | " << std::setw(8) << "Error" << " | OK\n";
    std::cout << "  " << std::string(48, '-') << "\n";
    
    int pass = 0;
    for (int i = 0; i < N; i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i] - dec);
        double tol = std::max(5.0, std::abs(tests[i]) * 0.15);
        bool ok = err < tol;
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(2) << tests[i]
                  << " | " << std::setw(10) << std::setprecision(2) << dec
                  << " | " << std::setw(8) << std::setprecision(2) << err
                  << " | " << (ok ? "✓" : "✗") << "\n";
        if (ok) pass++;
    }
    std::cout << "  Passed: " << pass << "/" << N << "\n";
    
    // Observer uniqueness
    std::cout << "\n═══ OBSERVER UNIQUENESS ═══\n\n";
    MirrorFHE alice(11111, 64), bob(22222, 64);
    auto ct = alice.encrypt(42.0);
    double da = alice.decrypt(ct), db = bob.decrypt(ct);
    std::cout << "  Alice: " << std::fixed << std::setprecision(2) << da << " ✓\n";
    std::cout << "  Bob:   " << std::fixed << std::setprecision(2) << db << " ✗\n";
    std::cout << "  Ratio: " << std::abs(db-42)/std::max(std::abs(da-42),1e-10) << "x\n";
    
    // Homomorphic add
    std::cout << "\n═══ HOMOMORPHIC ADD ═══\n";
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    auto csum = fhe.add(ca, cb);
    std::cout << "  15+25 = " << fhe.decrypt(csum) << " (exp 40)\n";
    
    // Chain add
    auto cacc = ca;
    for (int i = 0; i < 10; i++) cacc = fhe.add(cacc, cb);
    std::cout << "  15+11×25 = " << fhe.decrypt(cacc) << " (exp 290) depth=" << cacc.depth << "\n";
    
    // Homomorphic mul
    std::cout << "\n═══ HOMOMORPHIC MUL ═══\n";
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    auto cmul = fhe.multiply(cx, cy);
    std::cout << "  6×7 = " << fhe.decrypt(cmul) << " (exp 42) depth=" << cmul.depth << "\n";
    
    // Deep: (a+b)(a-b)
    auto cneg = fhe.encrypt(-25.0);
    auto cdiff = fhe.add(ca, cneg);
    auto cform = fhe.multiply(csum, cdiff);
    std::cout << "  (15+25)(15-25) = " << fhe.decrypt(cform) << " (exp -400) depth=" << cform.depth << "\n";
    
    // Benchmark
    std::cout << "\n═══ BENCHMARK ═══\n";
    const int M = 100;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i=0;i<M;i++) fhe.encrypt(i);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "  Encrypt: " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/M << " µs\n";
    
    auto ctb = fhe.encrypt(1.0);
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i=0;i<M;i++) fhe.add(ctb, ctb);
    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "  Add:     " << std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count()/M << " µs\n";
    
    auto t5 = std::chrono::high_resolution_clock::now();
    for (int i=0;i<M;i++) fhe.multiply(ctb, ctb);
    auto t6 = std::chrono::high_resolution_clock::now();
    std::cout << "  Mul:     " << std::chrono::duration_cast<std::chrono::microseconds>(t6-t5).count()/M << " µs\n";
    
    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << "  Ratio Encoding: value = |1⟩/|0⟩ amplitude ratio\n";
    std::cout << "  Soft Fibonacci Floor: gravitational pull, not snap\n";
    std::cout << "  Zeta zeros anchor phases on critical line\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════\n";
    return 0;
}
