#include <iostream>
#include <iomanip>
#include <cmath>
#include "../include/true_fhe.hpp"
using namespace true_fhe;

int main() {
    TrueFHE fhe(42);
    int pass = 0, fail = 0;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TRUE FHE — RLWE on φ-Polynomial Ring                       ║\n";
    std::cout << "║  Semantic Security | Secret Key | Gaussian Noise            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    // Encrypt/Decrypt (with noise — approximate)
    std::cout << "═══ ENCRYPT/DECRYPT ═══\n\n";
    double tests[] = {0, 1.618, 3.1416, 42, 100, -7.5, -100};
    for (double t : tests) {
        auto ct = fhe.encrypt(t);
        double dec = fhe.decrypt(ct);
        double err = std::abs(t - dec);
        bool ok = err < 0.5;  // Small noise expected
        if (ok) pass++; else fail++;
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(8) << t
                  << " → " << std::setw(12) << std::setprecision(4) << dec
                  << " | err=" << std::scientific << err << " " << (ok ? "✓" : "≈") << "\n";
    }
    
    // Homomorphic Add
    std::cout << "\n═══ HOMOMORPHIC ADD ═══\n\n";
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    auto csum = fhe.add(ca, cb);
    double r = fhe.decrypt(csum);
    bool ok = std::abs(r - 40.0) < 2.0;
    if (ok) pass++; else fail++;
    std::cout << "  15+25 = " << r << " (exp 40) " << (ok ? "✓" : "≈") << "\n";
    
    // Homomorphic Mul
    std::cout << "\n═══ HOMOMORPHIC MUL ═══\n\n";
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    auto cp = fhe.multiply(cx, cy);
    double rm = fhe.decrypt(cp);
    ok = std::abs(rm - 42.0) < 5.0;
    if (ok) pass++; else fail++;
    std::cout << "  6x7 = " << rm << " (exp 42) " << (ok ? "✓" : "≈") << "\n";
    
    // Semantic security: two encryptions of same value differ
    auto c1 = fhe.encrypt(42.0, 1);
    auto c2 = fhe.encrypt(42.0, 2);
    bool different = false;
    for (size_t i = 0; i < POLY_N; i++) {
        if (c1.a[i] != c2.a[i] || c1.b[i] != c2.b[i]) { different = true; break; }
    }
    std::cout << "\n═══ SEMANTIC SECURITY ═══\n\n";
    std::cout << "  Enc(42) != Enc(42): " << (different ? "YES ✓" : "NO ✗") << "\n";
    if (different) pass++; else fail++;
    
    std::cout << "\n══════════════════════════════════════════════════════\n";
    int total = pass + fail;
    std::cout << "  Tests: " << total << " | Pass: " << pass << " | Fail: " << fail;
    std::cout << (fail == 0 ? " | ✅ ALL PASS\n" : " | ⚠\n");
    std::cout << "══════════════════════════════════════════════════════\n";
    return fail > 0 ? 1 : 0;
}
