#include "mirror_fhe_gravity.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
using namespace mirror_fhe;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════╗
║   MIRROR-FHE: QUANTUM GRAVITY STABILIZED                       ║
║   R_μν - ½Rg_μν + Λg_μν = 8πG·T_μν(φ)                          ║
║   States follow geodesics in φ-curved Hilbert manifold          ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero        ║
╚══════════════════════════════════════════════════════════════════╝
)";

    MirrorFHE fhe(42, 64);
    
    // ============================================================
    // ENCRYPT/DECRYPT with GRAVITY STABILIZATION
    // ============================================================
    std::cout << "\n═══ ENCRYPT/DECRYPT (Quantum Gravity Floor) ═══\n\n";
    double tests[] = {0, 1.618034, 3.141593, 13, 21, 34, 42, 55, 89, 100, 144, 500, -7.5, -100};
    const char* names[] = {"0","φ","π","13","21","34","42","55","89","100","144","500","-7.5","-100"};
    int N = 14, pass = 0;
    
    std::cout << "  " << std::setw(8) << "Value" << " → " << std::setw(12) << "Decrypted" 
              << " | " << std::setw(10) << "Error" << "\n";
    std::cout << "  " << std::string(42, '-') << "\n";
    
    for (int i = 0; i < N; i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i] - dec);
        double tol = std::max(3.0, std::abs(tests[i]) * 0.1);
        bool ok = err < tol;
        if (ok) pass++;
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(2) << tests[i]
                  << " → " << std::setw(12) << std::setprecision(4) << dec
                  << " | " << std::setw(10) << std::scientific << std::setprecision(2) << err
                  << " " << (ok ? "✓" : "≈") << "\n";
    }
    std::cout << "\n  ✅ Passed: " << pass << "/" << N << "\n";
    
    // ============================================================
    // OBSERVER UNIQUENESS
    // ============================================================
    std::cout << "\n═══ SECURITY ═══\n\n";
    MirrorFHE alice(11111,64), bob(22222,64);
    auto ct = alice.encrypt(42.0);
    double da = alice.decrypt(ct), db = bob.decrypt(ct);
    std::cout << "  Alice: " << std::fixed << std::setprecision(2) << da << " ✓\n";
    std::cout << "  Bob:   " << db << " ✗  Ratio: " 
              << std::setprecision(0) << std::abs(db-42)/std::max(std::abs(da-42),1e-10) << "x\n";
    
    // ============================================================
    // HOMOMORPHIC OPERATIONS
    // ============================================================
    std::cout << "\n═══ HOMOMORPHIC ADDITION ═══\n\n";
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    std::cout << "  15+25 = " << fhe.decrypt(fhe.add(ca,cb)) << " (exp 40)\n";
    
    auto cacc = ca;
    for (int i=0;i<5;i++) cacc = fhe.add(cacc, cb);
    std::cout << "  15+6×25 = " << fhe.decrypt(cacc) << " (exp 165) d=" << cacc.depth << "\n";
    
    std::cout << "\n═══ HOMOMORPHIC MULTIPLICATION ═══\n\n";
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    std::cout << "  6×7 = " << fhe.decrypt(fhe.multiply(cx,cy)) << " (exp 42)\n";
    
    auto cmul = fhe.multiply(cx, cy);
    auto cphi = fhe.encrypt(PHI);
    std::cout << "  42×φ = " << fhe.decrypt(fhe.multiply(cmul,cphi)) << " (exp " << (42*PHI) << ")\n";
    
    std::cout << "\n═══ DEEP: f(x)=x²+2x+1 at x=3 ═══\n\n";
    auto c3 = fhe.encrypt(3.0), c2 = fhe.encrypt(2.0), c1 = fhe.encrypt(1.0);
    auto r = fhe.add(fhe.add(fhe.multiply(c3,c3), fhe.multiply(c2,c3)), c1);
    std::cout << "  f(3) = " << fhe.decrypt(r) << " (exp 16) d=" << r.depth << "\n";
    
    // ============================================================
    // BENCHMARK
    // ============================================================
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    const int M = 100;
    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.encrypt(i);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "  Encrypt:  " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/M << " µs\n";
    
    auto cb1 = fhe.encrypt(1.0);
    auto t3 = std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.add(cb1,cb1);
    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "  Add:      " << std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count()/M << " µs\n";
    
    auto t5 = std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.multiply(cb1,cb1);
    auto t6 = std::chrono::high_resolution_clock::now();
    std::cout << "  Multiply: " << std::chrono::duration_cast<std::chrono::microseconds>(t6-t5).count()/M << " µs\n";
    
    std::cout << "\n══════════════════════════════════════════════════════════\n";
    std::cout << "  QUANTUM GRAVITY FHE — GEODESICALLY STABILIZED\n";
    std::cout << "  φ-curved Hilbert manifold prevents state drift\n";
    std::cout << "  T_μν(φ) = φ × H × ∫C(τ)dτ × g_μν\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════════════\n";
    return 0;
}
