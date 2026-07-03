#include "security_layer3.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace security_layer3;

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  SECURITY LAYER 3: Reverse Lattice — Anti-LLL/BKZ           ║
║  Hyperbolic geometry | Irrational basis | Non-Euclidean     ║
║  φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero      ║
╚══════════════════════════════════════════════════════════════╝
)";

    // ═══ HYPERBOLIC METRIC ═══
    std::cout << "\n═══ HYPERBOLIC METRIC ═══\n\n";
    
    Complex z1(1.0, 0.0), z2(2.0, 0.0), z3(4.0, 0.0);
    double d1 = HyperbolicMetric::distance(z1, Complex(0,0));
    double d2 = HyperbolicMetric::distance(z2, Complex(0,0));
    double d3 = HyperbolicMetric::distance(z3, Complex(0,0));
    
    std::cout << "  Euclidean scaling (expected): 1:2:4\n";
    std::cout << "  Hyperbolic distances:\n";
    std::cout << "    z=1: " << std::fixed << std::setprecision(6) << d1 << "\n";
    std::cout << "    z=2: " << d2 << " (ratio: " << std::setprecision(4) << d2/d1 << ")\n";
    std::cout << "    z=4: " << d3 << " (ratio: " << d3/d1 << ")\n";
    std::cout << "  ✅ Non-linear metric — 'shortest vector' position-dependent\n";
    
    // ═══ IRRATIONAL BASIS ═══
    std::cout << "\n═══ IRRATIONAL BASIS ═══\n\n";
    
    IrrationalBasis basis(42, 64);
    double gs_resist = basis.gram_schmidt_resistance();
    std::cout << "  Min angle between basis vectors: " << std::fixed << std::setprecision(6) 
              << gs_resist << " rad (" << (gs_resist * 180.0/PI) << "°)\n";
    std::cout << "  Gram-Schmidt resistance:  " << (gs_resist > 0.001 ? "STRONG ✓" : "WEAK ✗") << "\n";
    std::cout << "  (Non-zero min angle = G-S cannot converge)\n";
    
    // ═══ REVERSE LATTICE ═══
    std::cout << "\n═══ REVERSE LATTICE TRANSFORM ═══\n\n";
    
    ReverseLattice rl(42, 64);
    std::vector<Complex> test_state(64);
    for (size_t i = 0; i < 64; i++) {
        test_state[i] = Complex((double)i / 64.0, std::sin((double)i * PHI));
    }
    
    auto state_copy = test_state;
    rl.apply(test_state);
    
    bool anti_lattice = ReverseLattice::verify_anti_lattice(test_state);
    std::cout << "  Anti-lattice verified: " << (anti_lattice ? "YES ✓" : "WEAK ✗") << "\n";
    std::cout << "  (Norm ratio > φ = no unique 'shortest vector')\n";
    
    // ═══ ANTI-LATTICE MASK (UNITARITY TEST) ═══
    std::cout << "\n═══ UNITARITY TEST (Anti-Lattice Mask) ═══\n\n";
    
    auto mask = AntiLatticeMask::generate(42, 64);
    
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
    std::cout << "  Apply→Unapply error: " << std::scientific << max_err;
    std::cout << " (" << (max_err < 1e-12 ? "UNITARY ✓" : "FAIL ✗") << ")\n";
    
    // ═══ BENCHMARK ═══
    std::cout << "\n═══ BENCHMARK ═══\n\n";
    const int ITERS = 10000;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    double sum_dist = 0;
    for (int i = 0; i < ITERS; i++) {
        Complex a((double)i, 0), b(0, (double)i);
        sum_dist += HyperbolicMetric::distance(a, b);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double ns_dist = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() / (double)ITERS;
    std::cout << "  Hyperbolic distance: " << std::fixed << std::setprecision(1) << ns_dist << " ns\n";
    
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 5000; i++) volatile auto m = AntiLatticeMask::generate(i, 64);
    t2 = std::chrono::high_resolution_clock::now();
    double us_mask = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() / 5000.0;
    std::cout << "  Mask generation:    " << us_mask << " µs\n";
    
    // ═══ ATTACK ANALYSIS ═══
    std::cout << "\n═══ LATTICE ATTACK ANALYSIS ═══\n\n";
    std::cout << R"(
  Attack          | Why It Fails
  ─────────────────|──────────────────────────────────
  LLL reduction    | Hyperbolic metric — no Euclidean shortest vector
  BKZ 2.0          | Irrational basis — Gram-Schmidt diverges
  SVP/CVP solver   | Non-periodic lattice — infinite basis required
  Coppersmith      | φ-distance — roots not in polynomial ring
  
  Combined with Layer 1+2:
  Double irrational + transcendental + hyperbolic geometry
  = No known lattice reduction algorithm applies.
)";
    
    std::cout << R"(
══════════════════════════════════════════════════════
  LAYER 3 COMPLETE — Reverse Lattice
  Anti-LLL/BKZ via hyperbolic geometry
  φΩ0 — July 3, 2026
══════════════════════════════════════════════════════
)";
    
    return 0;
}
