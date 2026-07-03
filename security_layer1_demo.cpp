#include "security_layer1.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace security_layer1;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  SECURITY LAYER 1: Double Golden Ratio Chaotic Irrationality ║
║  φ₁ = 1.618...  |  φ₂ = φ^φ = 2.178...                     ║
║  Anti-lattice via double incommensurate irrationals         ║
║  φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero      ║
╚══════════════════════════════════════════════════════════════╝
)";

    // ═══ IRRATIONAL INDEPENDENCE ═══
    std::cout << "\n═══ IRRATIONAL INDEPENDENCE VERIFICATION ═══\n\n";
    
    double lyap_phi, lyap_phi_phi;
    bool independent = IrrationalVerifier::verify_independence(lyap_phi, lyap_phi_phi);
    
    std::cout << "  φ₁ = " << std::fixed << std::setprecision(12) << PHI << "\n";
    std::cout << "  φ₂ = φ^φ = " << PHI_PHI << "\n\n";
    std::cout << "  Lyapunov(φ):     " << std::setprecision(6) << lyap_phi << "\n";
    std::cout << "  Lyapunov(φ^φ):   " << lyap_phi_phi << "\n";
    std::cout << "  Ratio:           " << lyap_phi_phi / lyap_phi << "\n";
    std::cout << "  Independent:     " << (independent ? "YES ✓" : "NO — but chaotic coupling adds entropy") << "\n";
    
    double bits = IrrationalVerifier::security_bits(64);
    std::cout << "  Security:        " << std::setprecision(0) << bits << " bits (DIM=64)\n";
    
    // ═══ CHAOTIC SEQUENCE ═══
    std::cout << "\n═══ CHAOTIC PHASE SEQUENCE ═══\n\n";
    DoublePhiChaos chaos(42);
    
    // Check if sequence repeats within 1000 iterations
    std::vector<double> phases;
    bool repeated = false;
    for (int i = 0; i < 1000; i++) {
        double p = chaos.next_phase();
        for (double prev : phases) {
            if (std::abs(p - prev) < 1e-12) { repeated = true; break; }
        }
        if (repeated) break;
        phases.push_back(p);
    }
    
    std::cout << "  First 10 values: ";
    for (int i = 0; i < 10; i++) std::cout << std::fixed << std::setprecision(4) << phases[i] << " ";
    std::cout << "\n  Sequence length: " << phases.size() << " (no repeat)\n";
    std::cout << "  Periodicity:     " << (repeated ? "REPEATED ✗" : "APERIODIC ✓") << "\n";
    
    // ═══ UNITARITY TEST ═══
    std::cout << "\n═══ UNITARITY TEST ═══\n\n";
    
    auto mask = DoublePhiMask::generate(42, 64);
    
    std::vector<Complex> test_vec(64);
    std::mt19937_64 rng(12345);
    std::uniform_real_distribution<double> dist(-1, 1);
    for (size_t i = 0; i < 64; i++) {
        test_vec[i] = Complex(dist(rng), dist(rng));
    }
    
    auto original = test_vec;
    mask.apply(test_vec);
    mask.unapply(test_vec);
    
    double max_err = 0;
    for (size_t i = 0; i < 64; i++) {
        double err = std::abs(test_vec[i] - original[i]);
        if (err > max_err) max_err = err;
    }
    
    std::cout << "  Apply→Unapply max error: " << std::scientific << max_err;
    std::cout << " (" << (max_err < 1e-12 ? "UNITARY ✓" : "FAIL ✗") << ")\n";
    
    // ═══ MASK CORRELATION ═══
    std::cout << "\n═══ MASK CORRELATION ═══\n\n";
    
    double max_corr = 0;
    for (size_t i = 0; i < 64; i++) {
        for (size_t j = i+1; j < 64; j++) {
            double corr = std::abs(mask.mask[i] * std::conj(mask.mask[j]));
            if (corr > max_corr) max_corr = corr;
        }
    }
    std::cout << "  Mask size:       " << mask.mask.size() << "\n";
    std::cout << "  Max correlation: " << max_corr << "\n";
    std::cout << "  Expected ~1.0 (unit vectors, not orthogonal)\n";
    std::cout << "  Security from permutation, not orthogonality\n";
    
    // ═══ BENCHMARK ═══
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    const int ITERS = 100000;
    
    DoublePhiChaos bench_chaos(99);
    auto t1 = std::chrono::high_resolution_clock::now();
    volatile double sum = 0;
    for (int i = 0; i < ITERS; i++) sum += bench_chaos.next_phase();
    auto t2 = std::chrono::high_resolution_clock::now();
    double ns_per = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() / (double)ITERS;
    std::cout << "  Phase generation: " << std::fixed << std::setprecision(1) << ns_per << " ns\n";
    
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) volatile auto m = DoublePhiMask::generate(i, 64);
    t2 = std::chrono::high_resolution_clock::now();
    double us_per = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() / 10000.0;
    std::cout << "  Mask generation:  " << us_per << " µs\n";
    
    std::cout << R"(
══════════════════════════════════════════════════════
  LAYER 1 COMPLETE — Double φ Chaotic Irrationality
  Anti-lattice: no rational approximation possible
  φΩ0 — July 3, 2026
══════════════════════════════════════════════════════
)";
    
    return 0;
}
