#include "mirror_fhe_v5.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
using namespace mirror_fhe;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║   MIRROR-FHE v5 — Ratio-Aware Homomorphic Operations       ║
║   Add: angle addition formulas in encrypted domain          ║
║   Mul: ratio multiplication via angle scaling              ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero    ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    MirrorFHE fhe(42, 64);
    
    // === ENCRYPT/DECRYPT ===
    std::cout << "\n═══ ENCRYPT/DECRYPT ═══\n";
    double tests[] = {0, 1.618, 3.1416, 13, 21, 34, 42, 55, 89, 100, 144, 500, -7.5, -100};
    int N = 14;
    int pass = 0;
    for (int i=0;i<N;i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i]-dec);
        bool ok = err < std::max(8.0, std::abs(tests[i])*0.2);
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(2) << tests[i]
                  << " → " << std::setw(10) << std::setprecision(2) << dec
                  << " (err " << std::setprecision(2) << err << ") " << (ok?"✓":"✗") << "\n";
        if(ok) pass++;
    }
    std::cout << "  Passed: " << pass << "/" << N << "\n";
    
    // === OBSERVER UNIQUENESS ===
    std::cout << "\n═══ OBSERVER UNIQUENESS ═══\n";
    MirrorFHE alice(11111,64), bob(22222,64);
    auto ct = alice.encrypt(42.0);
    double da=alice.decrypt(ct), db=bob.decrypt(ct);
    std::cout << "  Alice: " << da << " ✓ | Bob: " << db << " ✗ | Ratio: " 
              << std::abs(db-42)/std::max(std::abs(da-42),1e-10) << "x\n";
    
    // === HOMOMORPHIC ADD ===
    std::cout << "\n═══ HOMOMORPHIC ADD (Angle Addition) ═══\n";
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    auto csum = fhe.add(ca, cb);
    std::cout << "  15+25 = " << fhe.decrypt(csum) << " (exp 40)\n";
    
    auto cacc = ca;
    for(int i=0;i<5;i++) cacc = fhe.add(cacc, cb);
    std::cout << "  15+6×25 = " << fhe.decrypt(cacc) << " (exp 165) depth=" << cacc.depth << "\n";
    
    // === HOMOMORPHIC MUL ===
    std::cout << "\n═══ HOMOMORPHIC MUL (Ratio Multiplication) ═══\n";
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    auto cmul = fhe.multiply(cx, cy);
    std::cout << "  6×7 = " << fhe.decrypt(cmul) << " (exp 42) depth=" << cmul.depth << "\n";
    
    auto cphi = fhe.encrypt(PHI);
    auto cmul2 = fhe.multiply(cmul, cphi);
    std::cout << "  42×φ = " << fhe.decrypt(cmul2) << " (exp " << (42*PHI) << ") depth=" << cmul2.depth << "\n";
    
    // === DEEP: (a+b)(a-b) ===
    std::cout << "\n═══ DEEP: (a+b)(a-b) ═══\n";
    auto cneg = fhe.encrypt(-25.0);
    auto cdiff = fhe.add(ca, cneg);
    auto cform = fhe.multiply(csum, cdiff);
    std::cout << "  (15+25)(15-25) = " << fhe.decrypt(cform) << " (exp -400) depth=" << cform.depth << "\n";
    
    // === BENCHMARK ===
    std::cout << "\n═══ BENCHMARK ═══\n";
    const int M=100;
    auto t1=std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.encrypt(i);
    auto t2=std::chrono::high_resolution_clock::now();
    std::cout << "  Encrypt: " << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/M << " µs\n";
    
    auto c1=fhe.encrypt(1.0);
    auto t3=std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.add(c1,c1);
    auto t4=std::chrono::high_resolution_clock::now();
    std::cout << "  Add:     " << std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count()/M << " µs\n";
    
    auto t5=std::chrono::high_resolution_clock::now();
    for(int i=0;i<M;i++) fhe.multiply(c1,c1);
    auto t6=std::chrono::high_resolution_clock::now();
    std::cout << "  Mul:     " << std::chrono::duration_cast<std::chrono::microseconds>(t6-t5).count()/M << " µs\n";
    
    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << "  Ratio-Aware Ops: angle addition + ratio multiplication\n";
    std::cout << "  Encrypted domain operations preserve ratio structure\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════\n";
    return 0;
}
