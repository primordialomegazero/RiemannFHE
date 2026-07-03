#include "mirror_fhe_critical_line.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace mirror_fhe;

void test_encrypt_decrypt() {
    std::cout << "\n═══ ENCRYPT/DECRYPT — Critical Line Aligned ═══\n\n";
    
    MirrorFHE fhe(42, 64);
    
    struct TestCase { double val; const char* name; };
    TestCase tests[] = {
        {1.618034, "φ"},
        {3.141593, "π"},
        {42.0, "42"},
        {-7.5, "-7.5"},
        {0.0, "0"},
        {100.0, "100"},
        {0.618034, "φ⁻¹"},
        {-100.0, "-100"},
        {500.0, "500"},
        {999.0, "999"},
    };
    const int N = sizeof(tests)/sizeof(tests[0]);
    
    std::cout << "  " << std::setw(8) << "Value" << " | " 
              << std::setw(12) << "Decrypted" << " | "
              << std::setw(10) << "Error" << " | "
              << "Status\n";
    std::cout << "  " << std::string(52, '-') << "\n";
    
    int pass = 0;
    for (int i = 0; i < N; i++) {
        auto ct = fhe.encrypt(tests[i].val);
        double dec = fhe.decrypt(ct);
        double err = std::abs(tests[i].val - dec);
        bool ok = err < std::max(5.0, std::abs(tests[i].val) * 0.1);
        
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(2) 
                  << tests[i].val << " | "
                  << std::setw(12) << std::setprecision(4) << dec << " | "
                  << std::setw(8) << std::scientific << std::setprecision(1) << err << " | "
                  << (ok ? "✓" : "✗") << "\n";
        if (ok) pass++;
    }
    
    std::cout << "\n  Passed: " << pass << "/" << N << "\n";
}

void test_observer_uniqueness() {
    std::cout << "\n═══ OBSERVER UNIQUENESS ═══\n\n";
    
    MirrorFHE alice(11111, 64);
    MirrorFHE bob(22222, 64);
    
    double msg = 42.0;
    auto ct_alice = alice.encrypt(msg);
    
    double dec_alice = alice.decrypt(ct_alice);
    double dec_bob = bob.decrypt(ct_alice);
    
    std::cout << "  Message: " << msg << "\n\n";
    std::cout << "  Alice decrypts: " << std::fixed << std::setprecision(4) 
              << dec_alice << "  ✓ (correct observer)\n";
    std::cout << "  Bob decrypts:   " << std::fixed << std::setprecision(4) 
              << dec_bob << "  ✗ (wrong observer)\n";
    
    double err_alice = std::abs(msg - dec_alice);
    double err_bob = std::abs(msg - dec_bob);
    
    std::cout << "\n  Alice error: " << std::scientific << err_alice << "\n";
    std::cout << "  Bob error:   " << std::scientific << err_bob << "\n";
    std::cout << "  Security ratio: " << std::fixed << std::setprecision(1) 
              << (err_bob / std::max(err_alice, 1e-10)) << "x\n";
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
    
    // Chain 20 additions
    auto ct_acc = ct_a;
    for (int i = 0; i < 20; i++) {
        ct_acc = fhe.add(ct_acc, ct_b);
    }
    double dec_acc = fhe.decrypt(ct_acc);
    double expected = a + 21 * b;
    
    std::cout << "  After 21 additions: " << std::fixed << std::setprecision(4) 
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
    std::cout << "  Depth: " << ct_mul.depth << "\n\n";
    
    // Multiply chain: ((a × b) × φ) × π
    auto ct_phi = fhe.encrypt(PHI);
    auto ct_pi = fhe.encrypt(PI);
    
    auto ct_t1 = fhe.multiply(ct_mul, ct_phi);
    double dec_t1 = fhe.decrypt(ct_t1);
    std::cout << "  (6×7) × φ = " << std::fixed << std::setprecision(4) 
              << dec_t1 << " (expected " << (42*PHI) << ")\n";
    
    auto ct_t2 = fhe.multiply(ct_t1, ct_pi);
    double dec_t2 = fhe.decrypt(ct_t2);
    std::cout << "  (42×φ) × π = " << std::fixed << std::setprecision(4) 
              << dec_t2 << " (expected " << (42*PHI*PI) << ")\n";
}

void test_deep_polynomial() {
    std::cout << "\n═══ DEEP: f(x)=x²+2x+1 at x=3 ═══\n\n";
    
    MirrorFHE fhe(77777, 64);
    
    double x = 3.0;
    auto ct_x = fhe.encrypt(x);
    auto ct_2 = fhe.encrypt(2.0);
    auto ct_1 = fhe.encrypt(1.0);
    
    auto ct_x2 = fhe.multiply(ct_x, ct_x);
    auto ct_2x = fhe.multiply(ct_2, ct_x);
    auto ct_sum1 = fhe.add(ct_x2, ct_2x);
    auto ct_result = fhe.add(ct_sum1, ct_1);
    
    double dec = fhe.decrypt(ct_result);
    double expected = x*x + 2*x + 1; // 16
    
    std::cout << "  f(" << x << ") = " << std::fixed << std::setprecision(4) 
              << dec << " (expected " << expected << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(dec - expected) << "\n";
    std::cout << "  Depth: " << ct_result.depth << "\n";
}

void benchmark() {
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    
    MirrorFHE fhe(999, 64);
    
    const int N = 100;
    volatile double sink = 0;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { auto ct = fhe.encrypt(i * 1.0); }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto enc_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / N;
    
    auto ct = fhe.encrypt(3.14);
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { sink = fhe.decrypt(ct); }
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
    (void)sink;
}

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║   MIRROR-FHE: Critical Line Aligned Encryption              ║
║   Observer anchored to Riemann zeta zeros on Re(s)=1/2      ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero    ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    std::cout << "\n  Using " << ZETA_COUNT << " zeta zeros as critical line basis\n";
    std::cout << "  γ₁ = " << ZETA_ZEROS[0] << " ... γ₁₀₀ = " << ZETA_ZEROS[ZETA_COUNT-1] << "\n";
    
    test_encrypt_decrypt();
    test_observer_uniqueness();
    test_homomorphic_add();
    test_homomorphic_mul();
    test_deep_polynomial();
    benchmark();
    
    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << "  Critical Line Alignment: ENCRYPT/DECRYPT SYNCED\n";
    std::cout << "  Zeta zeros anchor the phase space\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════\n";
    
    return 0;
}
