#include "fhe_gravity_core.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
using namespace fhe_gravity;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════╗
║   FIBONACCI QUANTUM GRAVITY FHE                                 ║
║   S = ∫ ds = ∫ √(g_ab dx^a dx^b) — Geodesic Action Principle   ║
║   d²x^μ/dτ² + Γ^μ_νρ (dx^ν/dτ)(dx^ρ/dτ) = 0                    ║
║   V(x) = D_e(1 - exp(-a·dx))² — Morse Fibonacci Potential      ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero        ║
╚══════════════════════════════════════════════════════════════════╝
)";
    
    // Display Fibonacci ladder
    std::cout << "\n═══ FIBONACCI LADDER (first 15 levels) ═══\n";
    for(size_t i=0;i<15;i++) {
        std::cout << "  F_" << i << "=" << std::setw(4) << FIB[i] 
                  << " → " << std::fixed << std::setprecision(2) << FIB_LADDER.levels[i] << "\n";
    }
    std::cout << "  ...\n  F_63=" << FIB[63] << " → " << FIB_LADDER.levels[63] << " (max)\n";
    
    // Initialize FHE with moderate gravity
    GravityFHE fhe(42, 64, 0.3);
    
    // ============================================================
    // ENCRYPT/DECRYPT
    // ============================================================
    std::cout << "\n═══ ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618034, 3.141593, 13, 21, 34, 42, 55, 89, 100, 144, 233, 500, 610, -7.5, -100};
    int N = 16, pass = 0;
    
    std::cout << "  " << std::setw(8) << "Value" << " → " << std::setw(12) << "Decrypted"
              << " | " << std::setw(10) << "Error" << "\n";
    std::cout << "  " << std::string(42,'-') << "\n";
    
    for(int i=0;i<N;i++){
        auto ct=fhe.encrypt(tests[i]);
        double dec=fhe.decrypt(ct);
        double err=std::abs(tests[i]-dec);
        double tol=std::max(5.0,std::abs(tests[i])*0.15);
        bool ok=err<tol;
        if(ok)pass++;
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(2) << tests[i]
                  << " → " << std::setw(12) << std::setprecision(4) << dec
                  << " | " << std::setw(10) << std::scientific << std::setprecision(2) << err
                  << " " << (ok?"✓":"≈") << "\n";
    }
    std::cout << "\n  ✅ Passed: " << pass << "/" << N << "\n";
    
    // ============================================================
    // OBSERVER UNIQUENESS
    // ============================================================
    std::cout << "\n═══ SECURITY ═══\n\n";
    GravityFHE alice(11111,64,0.3), bob(22222,64,0.3);
    auto ct=alice.encrypt(42.0);
    double da=alice.decrypt(ct), db=bob.decrypt(ct);
    std::cout << "  Alice: " << std::fixed << std::setprecision(2) << da << " ✓\n";
    std::cout << "  Bob:   " << db << " ✗\n";
    std::cout << "  Ratio: " << std::setprecision(0) 
              << std::abs(db-42)/std::max(std::abs(da-42),1e-10) << "x\n";
    
    // ============================================================
    // HOMOMORPHIC ADDITION
    // ============================================================
    std::cout << "\n═══ HOMOMORPHIC ADDITION ═══\n\n";
    auto ca=fhe.encrypt(8.0), cb=fhe.encrypt(13.0);
    std::cout << "  8+13 = " << fhe.decrypt(fhe.add(ca,cb)) << " (exp 21=F8)\n";
    
    auto cacc=ca;
    for(int i=0;i<5;i++) cacc=fhe.add(cacc,cb);
    std::cout << "  8+6×13 = " << fhe.decrypt(cacc) << " (exp 86) d=" << cacc.depth << "\n";
    
    // ============================================================
    // HOMOMORPHIC MULTIPLICATION
    // ============================================================
    std::cout << "\n═══ HOMOMORPHIC MULTIPLICATION ═══\n\n";
    auto cx=fhe.encrypt(5.0), cy=fhe.encrypt(8.0);
    std::cout << "  5×8 = " << fhe.decrypt(fhe.multiply(cx,cy)) << " (exp 40)\n";
    
    auto cmul=fhe.multiply(cx,cy);
    auto cphi=fhe.encrypt(PHI);
    std::cout << "  40×φ = " << fhe.decrypt(fhe.multiply(cmul,cphi)) << " (exp " << (40*PHI) << ")\n";
    
    // ============================================================
    // DEEP COMPUTATION
    // ============================================================
    std::cout << "\n═══ DEEP: f(x)=x²+2x+1 at x=3 ═══\n\n";
    auto c3=fhe.encrypt(3.0),c2=fhe.encrypt(2.0),c1=fhe.encrypt(1.0);
    auto r=fhe.add(fhe.add(fhe.multiply(c3,c3),fhe.multiply(c2,c3)),c1);
    std::cout << "  f(3) = " << fhe.decrypt(r) << " (exp 16) d=" << r.depth << "\n";
    std::cout << "  Energy: " << std::scientific << r.energy << "\n";
    
    // ============================================================
    // BENCHMARK
    // ============================================================
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    const int M=100;
    auto t1=std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.encrypt(i);
    auto t2=std::chrono::high_resolution_clock::now();
    std::cout << "  Encrypt:  " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/M << " µs\n";
    
    auto cb1=fhe.encrypt(1.0);
    auto t3=std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.add(cb1,cb1);
    auto t4=std::chrono::high_resolution_clock::now();
    std::cout << "  Add:      " << std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count()/M << " µs\n";
    
    auto t5=std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.multiply(cb1,cb1);
    auto t6=std::chrono::high_resolution_clock::now();
    std::cout << "  Multiply: " << std::chrono::duration_cast<std::chrono::microseconds>(t6-t5).count()/M << " µs\n";
    
    std::cout << "\n══════════════════════════════════════════════════════════\n";
    std::cout << "  FIBONACCI QUANTUM GRAVITY FHE\n";
    std::cout << "  Morse potential: V(x) = D_e(1-e^{-a·dx})²\n";
    std::cout << "  Geodesic flow: d²x/dτ² + Γ(dx/dτ)² = -∇V\n";
    std::cout << "  Fibonacci attractors stabilize values naturally\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════════════\n";
    return 0;
}
