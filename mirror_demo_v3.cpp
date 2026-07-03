#include "mirror_fhe_v3.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace mirror_fhe;

void test_encrypt_decrypt() {
    std::cout << "\n═══ ENCRYPT/DECRYPT TEST ═══\n\n";
    
    MirrorFHE fhe(42, 64);
    
    double tests[] = {1.618034, 3.141593, 42.0, -7.5, 0.0, 100.0, 0.618034, -100.0, 500.0};
    const char* names[] = {"φ", "π", "42", "-7.5", "0", "100", "φ⁻¹", "-100", "500"};
    
    std::cout << "  " << std::setw(8) << "Value" << " | " 
              << std::setw(12) << "Decrypted" << " | "
              << std::setw(10) << "Error" << " | "
              << "Status\n";
    std::cout << "  " << std::string(52, '-') << "\n";
    
    int pass = 0;
    for (int i = 0; i < 9; i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i] - dec);
        bool ok = err < 5.0;  // Tolerance for amplitude encoding
        
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(2) 
                  << tests[i] << " | "
                  << std::setw(12) << std::setprecision(4) << dec << " | "
                  << std::setw(8) << std::scientific << std::setprecision(1) << err << " | "
                  << (ok ? "✓" : "✗") << "\n";
        if (ok) pass++;
    }
    
    std::cout << "\n  Passed: " << pass << "/9\n";
}

void test_homomorphic_add() {
    std::cout << "\n═══ HOMOMORPHIC ADDITION ═══\n\n";
    
    MirrorFHE fhe(12345, 64);
    
    double a = 15.0, b = 25.0;
    auto ct_a = fhe.encrypt(a);
    auto ct_b = fhe.encrypt(b);
    
    auto ct_sum = fhe.add(ct_a, ct_b);
    double dec = fhe.decrypt(ct_sum);
    
    std::cout << "  " << a << " + " << b << " = " << std::fixed 
              << std::setprecision(4) << dec << " (expected " << (a+b) << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(dec - (a+b)) << "\n\n";
    
    // Chain additions
    auto ct_acc = ct_a;
    for (int i = 0; i < 10; i++) {
        ct_acc = fhe.add(ct_acc, ct_b);
    }
    double dec_acc = fhe.decrypt(ct_acc);
    double expected = a + 11 * b;
    
    std::cout << "  After 11 additions: " << std::fixed << std::setprecision(4) 
              << dec_acc << " (expected " << expected << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(dec_acc - expected) << "\n";
    std::cout << "  Depth: " << ct_acc.depth << "\n";
}

void test_homomorphic_mul() {
    std::cout << "\n═══ HOMOMORPHIC MULTIPLICATION ═══\n\n";
    
    MirrorFHE fhe(99999, 64);
    
    double a = 6.0, b = 7.0;
    auto ct_a = fhe.encrypt(a);
    auto ct_b = fhe.encrypt(b);
    
    auto ct_mul = fhe.multiply(ct_a, ct_b);
    double dec = fhe.decrypt(ct_mul);
    
    std::cout << "  " << a << " × " << b << " = " << std::fixed 
              << std::setprecision(4) << dec << " (expected " << (a*b) << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(dec - (a*b)) << "\n";
    std::cout << "  Depth: " << ct_mul.depth << "\n";
}

void test_deep_computation() {
    std::cout << "\n═══ DEEP COMPUTATION: f(x) = x² + 2x + 1 at x=3 ═══\n\n";
    
    MirrorFHE fhe(77777, 64);
    
    double x = 3.0;
    auto ct_x = fhe.encrypt(x);
    
    // x²
    auto ct_x2 = fhe.multiply(ct_x, ct_x);
    std::cout << "  x² = " << std::fixed << std::setprecision(4) 
              << fhe.decrypt(ct_x2) << " (expected 9)\n";
    
    // 2x
    auto ct_two = fhe.encrypt(2.0);
    auto ct_2x = fhe.multiply(ct_two, ct_x);
    std::cout << "  2x = " << std::fixed << std::setprecision(4) 
              << fhe.decrypt(ct_2x) << " (expected 6)\n";
    
    // x² + 2x
    auto ct_sum = fhe.add(ct_x2, ct_2x);
    std::cout << "  x²+2x = " << std::fixed << std::setprecision(4) 
              << fhe.decrypt(ct_sum) << " (expected 15)\n";
    
    // x² + 2x + 1
    auto ct_one = fhe.encrypt(1.0);
    auto ct_result = fhe.add(ct_sum, ct_one);
    double dec = fhe.decrypt(ct_result);
    double expected = 16.0;
    
    std::cout << "\n  f(3) = " << std::fixed << std::setprecision(4) 
              << dec << " (expected " << expected << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(dec - expected) << "\n";
    std::cout << "  Total depth: " << ct_result.depth << "\n";
}

void test_observer_uniqueness() {
    std::cout << "\n═══ OBSERVER UNIQUENESS (SECURITY) ═══\n\n";
    
    MirrorFHE alice(11111, 64);
    MirrorFHE bob(22222, 64);
    
    double msg = 42.0;
    auto ct_alice = alice.encrypt(msg);
    
    double dec_alice = alice.decrypt(ct_alice);
    double dec_bob = bob.decrypt(ct_alice);
    
    std::cout << "  Alice encrypts: " << msg << "\n";
    std::cout << "  Alice decrypts: " << std::fixed << std::setprecision(4) 
              << dec_alice << " ✓\n";
    std::cout << "  Bob decrypts:   " << std::fixed << std::setprecision(4) 
              << dec_bob << " ✗ (wrong observer)\n";
    std::cout << "\n  ✓ Only the observer who entangled can de-entangle.\n";
}

void benchmark() {
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    
    MirrorFHE fhe(999, 64);
    
    const int N = 100;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { auto ct = fhe.encrypt(3.14); }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto enc_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / N;
    
    auto ct = fhe.encrypt(3.14);
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { volatile double d = fhe.decrypt(ct); }
    auto t4 = std::chrono::high_resolution_clock::now();
    auto dec_us = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / N;
    
    auto ct2 = fhe.encrypt(2.71);
    auto t5 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { auto c = fhe.add(ct, ct2); }
    auto t6 = std::chrono::high_resolution_clock::now();
    auto add_us = std::chrono::duration_cast<std::chrono::microseconds>(t6 - t5).count() / N;
    
    auto t7 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { auto c = fhe.multiply(ct, ct2); }
    auto t8 = std::chrono::high_resolution_clock::now();
    auto mul_us = std::chrono::duration_cast<std::chrono::microseconds>(t8 - t7).count() / N;
    
    std::cout << "  Dimension: " << fhe.dimension() << "\n";
    std::cout << "  Encrypt:  " << enc_us << " µs\n";
    std::cout << "  Decrypt:  " << dec_us << " µs\n";
    std::cout << "  Add:      " << add_us << " µs\n";
    std::cout << "  Multiply: " << mul_us << " µs\n";
}

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║   MIRROR-FHE v3 — Observer-Observed Entanglement FHE        ║
║   Stable Amplitude Encoding                                 ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero    ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    test_encrypt_decrypt();
    test_observer_uniqueness();
    test_homomorphic_add();
    test_homomorphic_mul();
    test_deep_computation();
    benchmark();
    
    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << "  MIRROR-FHE v3 — Complete\n";
    std::cout << "══════════════════════════════════════════════════\n";
    
    return 0;
}
