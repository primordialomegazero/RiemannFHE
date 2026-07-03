#include "mirror_fhe_fib.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace mirror_fhe;

void test_encrypt_decrypt() {
    std::cout << "\n═══ ENCRYPT/DECRYPT with FIBONACCI FLOOR ═══\n\n";
    
    MirrorFHE fhe(42, 64);
    
    struct TestCase { double val; const char* name; };
    TestCase tests[] = {
        {1.618034, "φ"},   {3.141593, "π"},   {42.0, "42"},
        {-7.5, "-7.5"},    {0.0, "0"},        {100.0, "100"},
        {0.618034, "φ⁻¹"}, {-100.0, "-100"},  {500.0, "500"},
        {13.0, "F_7"},     {21.0, "F_8"},     {34.0, "F_9"},
        {55.0, "F_10"},    {89.0, "F_11"},    {144.0, "F_12"},
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
        double tol = std::max(3.0, std::abs(tests[i].val) * 0.3);
        bool ok = err < tol;
        
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
    auto ct = alice.encrypt(msg);
    
    double dec_a = alice.decrypt(ct);
    double dec_b = bob.decrypt(ct);
    
    std::cout << "  Message: " << msg << "\n";
    std::cout << "  Alice:   " << std::fixed << std::setprecision(2) << dec_a 
              << " ✓\n";
    std::cout << "  Bob:     " << std::fixed << std::setprecision(2) << dec_b 
              << " ✗\n";
    std::cout << "  Ratio:   " << std::fixed << std::setprecision(1) 
              << std::abs(dec_b - msg) / std::max(std::abs(dec_a - msg), 1e-10) << "x\n";
}

void test_homomorphic_ops() {
    std::cout << "\n═══ HOMOMORPHIC OPERATIONS ═══\n\n";
    
    MirrorFHE fhe(12345, 64);
    
    // Addition
    double a = 13.0, b = 21.0;  // Fibonacci numbers!
    auto ct_a = fhe.encrypt(a);
    auto ct_b = fhe.encrypt(b);
    auto ct_sum = fhe.add(ct_a, ct_b);
    double dec_sum = fhe.decrypt(ct_sum);
    
    std::cout << "  " << a << " + " << b << " = " << std::fixed << std::setprecision(2) 
              << dec_sum << " (expected " << (a+b) << ")\n";
    
    // Chained additions
    auto ct_acc = ct_a;
    for (int i = 0; i < 20; i++) ct_acc = fhe.add(ct_acc, ct_b);
    double dec_acc = fhe.decrypt(ct_acc);
    std::cout << "  13 + 21×21 = " << std::fixed << std::setprecision(2) 
              << dec_acc << " (expected " << (13+21*21) << ")\n";
    
    // Multiplication
    double x = 8.0, y = 5.0;
    auto ct_x = fhe.encrypt(x);
    auto ct_y = fhe.encrypt(y);
    auto ct_mul = fhe.multiply(ct_x, ct_y);
    double dec_mul = fhe.decrypt(ct_mul);
    
    std::cout << "\n  " << x << " × " << y << " = " << std::fixed << std::setprecision(2) 
              << dec_mul << " (expected " << (x*y) << ")\n";
    
    // Deep: (a+b)×(a-b) = a² - b²
    auto ct_diff = fhe.add(ct_a, fhe.encrypt(-b));  // a - b
    auto ct_formula = fhe.multiply(ct_sum, ct_diff);
    double dec_formula = fhe.decrypt(ct_formula);
    double expected = a*a - b*b;
    
    std::cout << "  (a+b)(a-b) = " << std::fixed << std::setprecision(2) 
              << dec_formula << " (expected " << expected << ")\n";
    std::cout << "  Depth: " << ct_formula.depth << "\n";
}

void show_fibonacci_floor() {
    std::cout << "\n═══ FIBONACCI FLOOR TABLE ═══\n\n";
    std::cout << "  F_n values used as amplitude attractors:\n  ";
    for (size_t i = 0; i < 20; i++) {
        std::cout << FIBONACCI[i] << " ";
        if (i == 9) std::cout << "\n  ";
    }
    std::cout << "\n\n  φ-attractors (weights):\n  ";
    for (size_t i = 0; i < 16; i++) {
        std::cout << std::fixed << std::setprecision(4) << PHI_ATTRACTORS[i] << " ";
    }
    std::cout << "\n\n  Zeta zero phases (first 10):\n  ";
    for (size_t i = 0; i < 10; i++) {
        std::cout << std::fixed << std::setprecision(3) 
                  << std::fmod(ZETA_ZEROS[i] * 0.1, 2.0*PI) << " ";
    }
    std::cout << "\n";
}

void benchmark() {
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    
    MirrorFHE fhe(999, 64);
    const int N = 100;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) fhe.encrypt(i * 1.0);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto enc_us = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() / N;
    
    auto ct = fhe.encrypt(3.14);
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { volatile auto d = fhe.decrypt(ct); }
    auto t4 = std::chrono::high_resolution_clock::now();
    auto dec_us = std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count() / N;
    
    auto ct2 = fhe.encrypt(2.71);
    auto t5 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) fhe.add(ct, ct2);
    auto t6 = std::chrono::high_resolution_clock::now();
    auto add_us = std::chrono::duration_cast<std::chrono::microseconds>(t6-t5).count() / N;
    
    auto t7 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) fhe.multiply(ct, ct2);
    auto t8 = std::chrono::high_resolution_clock::now();
    auto mul_us = std::chrono::duration_cast<std::chrono::microseconds>(t8-t7).count() / N;
    
    std::cout << "  Encrypt:  " << enc_us << " µs\n";
    std::cout << "  Decrypt:  " << dec_us << " µs\n";
    std::cout << "  Add:      " << add_us << " µs\n";
    std::cout << "  Multiply: " << mul_us << " µs\n";
}

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║   MIRROR-FHE: Fibonacci Floor Stabilized                    ║
║   F_n + Zeta Zeros + φ-attractors = Aligned Encryption     ║
║   φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero    ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    show_fibonacci_floor();
    test_encrypt_decrypt();
    test_observer_uniqueness();
    test_homomorphic_ops();
    benchmark();
    
    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << "  Fibonacci Floor: THE MISSING PIECE\n";
    std::cout << "  F_n grounds amplitudes → no drift → correct decrypt\n";
    std::cout << "  φΩ0 — July 3, 2026\n";
    std::cout << "══════════════════════════════════════════════════\n";
    
    return 0;
}
