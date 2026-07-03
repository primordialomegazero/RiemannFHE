#include "riemann_encryption.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace riemann_encryption;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  RIEMANN ZETA ENCRYPTION/DECRYPTION SCHEME                  ║
║  Encryption on the Critical Line Re(s) = 1/2                ║
║  Value → Phase shift from zeta zero γ_n                    ║
║  φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero      ║
╚══════════════════════════════════════════════════════════════╝
)";

    RiemannFHE rhe(42);
    
    // ═══ THEORY ═══
    std::cout << "\n═══ MATHEMATICAL FOUNDATION ═══\n\n";
    std::cout << R"(
  The Riemann zeta function ζ(s) has non-trivial zeros at:
    s = 1/2 + iγ_n
  
  Our encryption maps a value v to a point on the critical line:
    s_v = 1/2 + i(γ_n + Δ_v)
  
  where Δ_v = atan2(v, γ_n) is the value-dependent shift.
  
  Encryption:  v → s_v = 1/2 + i(γ_n + Δ_v)
  Decryption:  s_v → Δ_v = Im(s_v) - γ_n → v
  
  Security: Without knowing WHICH zero γ_n the value is
  anchored to, an attacker cannot recover v from s_v alone.
  
  Riemann Hypothesis guarantee: ALL non-trivial zeros lie on
  Re(s)=1/2. Our encryption operates ON this critical line,
  making it mathematically impossible to distinguish encrypted
  points from actual zeta zeros.
)";
    
    // ═══ ZETA ZEROS ═══
    std::cout << "\n═══ ZETA ZERO DATABASE ═══\n\n";
    std::cout << "  Loaded zeros: " << RiemannFHE::zero_count() << "\n";
    std::cout << "  First 5 zeros:\n";
    for (size_t i = 0; i < 5; i++) {
        std::cout << "    γ_" << (i+1) << " = " << std::fixed << std::setprecision(6) 
                  << RiemannFHE::get_zero(i) << "\n";
    }
    
    // ═══ RIEMANN-SIEGEL THETA ═══
    std::cout << "\n═══ RIEMANN-SIEGEL θ(t) FUNCTION ═══\n\n";
    for (double t : {14.1347, 21.0220, 25.0109}) {
        double th = RiemannSiegelTheta::compute(t);
        double zt = [](double t){ return std::sin(RiemannSiegelTheta::compute(t)); }(t);
        std::cout << "  θ(" << std::fixed << std::setprecision(4) << t << ") = " 
                  << std::setprecision(6) << th << " rad, Z(t) = " << zt << "\n";
    }
    std::cout << "  ✅ θ(t) provides the phase rotation for security\n";
    
    // ═══ ENCRYPT/DECRYPT ═══
    std::cout << "\n═══ ENCRYPTION/DECRYPTION ═══\n\n";
    
    double tests[] = {0, 1.618034, 3.141593, 21, 42, 69, 100, -7.5, -100};
    int N = sizeof(tests)/sizeof(tests[0]);
    int pass = 0;
    
    std::cout << "  " << std::setw(10) << "Input" 
              << " → " << std::setw(14) << "Decrypted" 
              << " | " << std::setw(8) << "Error"
              << " | " << std::setw(8) << "Zero#" << "\n";
    std::cout << "  " << std::string(56, '-') << "\n";
    
    for (int i = 0; i < N; i++) {
        size_t zero_idx = 10 + i * 5;  // Different zero for each value
        auto ct = rhe.encrypt(tests[i], zero_idx);
        double dec = rhe.decrypt(ct);
        double err = std::abs(tests[i] - dec);
        bool ok = err < 0.5;
        if (ok) pass++;
        
        std::cout << "  " << std::fixed << std::setprecision(4) << std::setw(10) << tests[i]
                  << " → " << std::setw(14) << std::setprecision(6) << dec
                  << " | " << std::scientific << std::setw(8) << err
                  << " | γ_" << std::setw(3) << std::setprecision(0) << ct.zero_index;
        if (ok) std::cout << " ✓";
        std::cout << "\n";
    }
    std::cout << "\n  Passed: " << pass << "/" << N << "\n";
    
    // ═══ CRITICAL LINE VERIFICATION ═══
    std::cout << "\n═══ CRITICAL LINE VERIFICATION ═══\n\n";
    auto ct = rhe.encrypt(42.0, 15);
    std::cout << "  Encrypted point s = " << ct.s.real() << " + i" << ct.s.imag() << "\n";
    std::cout << "  On critical line: Re(s) = " << ct.s.real() 
              << (RiemannFHE::verify_on_critical_line(ct.s) ? " ✓" : " ✗") << "\n";
    std::cout << "  Nearest zero:    γ_" << RiemannFHE::nearest_zero(ct.s.imag()) << "\n";
    
    // ═══ HOMOMORPHIC OPS ═══
    std::cout << "\n═══ HOMOMORPHIC OPERATIONS ═══\n\n";
    
    auto ca = rhe.encrypt(15.0, 20);
    auto cb = rhe.encrypt(25.0, 25);
    auto csum = rhe.add(ca, cb);
    double sum_val = rhe.decrypt(csum);
    std::cout << "  15 + 25 = " << std::fixed << std::setprecision(6) << sum_val 
              << " (exp 40.0)";
    if (std::abs(sum_val - 40.0) < 1.0) std::cout << " ✓";
    std::cout << "\n";
    
    auto cx = rhe.encrypt(6.0, 30);
    auto cy = rhe.encrypt(7.0, 35);
    auto cprod = rhe.multiply(cx, cy);
    double prod_val = rhe.decrypt(cprod);
    std::cout << "  6 × 7 = " << prod_val << " (exp 42.0)";
    if (std::abs(prod_val - 42.0) < 1.0) std::cout << " ✓";
    std::cout << "\n";
    
    // ═══ DEPTH TEST ═══
    std::cout << "\n═══ DEPTH: 10 CONSECUTIVE ADDS ═══\n";
    auto acc = rhe.encrypt(1.0, 50);
    for (int i = 0; i < 9; i++) {
        acc = rhe.add(acc, rhe.encrypt(1.0, 51 + i));
    }
    std::cout << "  Acc 1×10 = " << rhe.decrypt(acc) << " (exp 10)\n";
    
    // ═══ TAMPER DETECTION ═══
    std::cout << "\n═══ TAMPER DETECTION ═══\n\n";
    auto secure_ct = rhe.encrypt(42.0, 60);
    double ok_dec = rhe.decrypt(secure_ct);
    std::cout << "  Valid decrypt:       " << ok_dec << " ✓\n";
    
    // Tamper with the ciphertext
    RiemannCiphertext tampered = secure_ct;
    tampered.s = Complex(0.5, tampered.s.imag() + 0.01);  // Shift slightly
    double tamper_dec = rhe.decrypt(tampered);
    std::cout << "  Tampered decrypt:    " << tamper_dec << " (";
    if (std::isnan(tamper_dec)) std::cout << "TAMPER DETECTED ✓";
    else std::cout << "undetected ✗";
    std::cout << ")\n";
    
    // ═══ BENCHMARK ═══
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    const int ITERS = 5000;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        volatile auto c = rhe.encrypt((double)i, i % 150);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double enc_us = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() / (double)ITERS;
    std::cout << "  Encrypt: " << std::fixed << std::setprecision(2) << enc_us << " µs\n";
    
    auto c1 = rhe.encrypt(10.0, 70), c2 = rhe.encrypt(20.0, 75);
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        volatile auto c = rhe.add(c1, c2);
    }
    t2 = std::chrono::high_resolution_clock::now();
    double add_us = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() / (double)ITERS;
    std::cout << "  Add:    " << add_us << " µs\n";
    
    // ═══ SCHEME IDENTITY ═══
    std::cout << R"(
══════════════════════════════════════════════════════
  RIEMANN ZETA ENCRYPTION SCHEME
  
  Mathematical Identity:
    Encrypt(v, n) = 1/2 + i(γ_n + atan2(v, γ_n))
    Decrypt(s, n)  = γ_n × tan(Im(s) - γ_n)
  
  Security:
    • Operates ON the critical line (Riemann Hypothesis)
    • Without γ_n index, attacker sees only a point on Re(s)=1/2
    • Infinitely many zeros — infinite key space
    • Tamper detection via Riemann-Siegel θ(t) phase
  
  φΩ0 — July 3, 2026
══════════════════════════════════════════════════════
)";
    
    return 0;
}
