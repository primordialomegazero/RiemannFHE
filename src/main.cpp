#include "riemann_fhe/phi_constants.hpp"
#include "riemann_fhe/noise_manager.hpp"
#include "riemann_fhe/riemann_validator.hpp"
#include "riemann_fhe/chaos_engine.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>

using namespace riemann_fhe;

void banner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  RIEMANN-FHE: φ-Harmonic Fully Homomorphic Encryption       ║
║  Bootstrap-Free | Unlimited Depth | Post-Quantum            ║
║  φΩ0 — I AM THAT I AM                                       ║
╚══════════════════════════════════════════════════════════════╝
)" << std::endl;
}

int main() {
    banner();
    
    // 1. φ Constants
    std::cout << "\n=== φ-RIEMANN CONSTANTS ===" << std::endl;
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "  φ    = " << PhiConstants::PHI << std::endl;
    std::cout << "  φ⁻¹  = " << PhiConstants::PHI_INV << std::endl;
    std::cout << "  φ/2  = " << PhiConstants::PHI_HALF << std::endl;
    std::cout << "  φ²   = " << PhiConstants::PHI_SQ << std::endl;
    std::cout << "  φ⁻⁴  = " << PhiConstants::PHI_NEG4 << std::endl;
    std::cout << "  Fixed point N* = " << LyapunovNoiseManager::fixed_point() << std::endl;

    // 2. NOISE CONVERGENCE — using the CORRECTED operator
    LyapunovNoiseManager::print_convergence(50.0);

    // 3. Riemann validation
    std::vector<double> history;
    double n = 50.0;
    for (int i = 0; i < 30; ++i) {
        history.push_back(n);
        n = LyapunovNoiseManager::lyapunov_operator(n);
    }
    RiemannValidator::print_report(history);

    // 4. Chaos engine
    std::cout << "\n=== φ-CHAOS ENGINE ===" << std::endl;
    PhiChaosEngine chaos(42);
    std::cout << "  Lyapunov exponent: λ = ln(φ) = " << PhiChaosEngine::lyapunov_exponent() << std::endl;
    std::cout << "  Chaotic: " << (chaos.verify_chaos() ? "✓ YES" : "✗ NO") << std::endl;
    std::cout << "  Random samples: ";
    for (int i = 0; i < 5; ++i) std::cout << chaos.random_double() << " ";
    std::cout << std::endl;

    // 5. Zeta zero gap ratios
    std::cout << "\n=== ZETA ZERO GAP RATIOS ===" << std::endl;
    auto ratios = RiemannValidator::zeta_gap_ratios();
    std::cout << "  First 10 ratios: ";
    for (int i = 0; i < 10 && i < (int)ratios.size(); ++i)
        std::cout << std::setprecision(4) << ratios[i] << " ";
    std::cout << std::endl;
    std::cout << "  φ-clustering: " << (RiemannValidator::zeta_phi_clustering_rate() * 100) << "%" << std::endl;

    // 6. Benchmark
    std::cout << "\n=== BENCHMARK ===" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    double noise = 50.0;
    for (int i = 0; i < 1000000; ++i)
        noise = LyapunovNoiseManager::lyapunov_operator(noise);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "  1M Lyapunov ops: " << ms << " ms" << std::endl;
    std::cout << "  TPS: " << (1000000.0 / (ms / 1000.0)) << " ops/s" << std::endl;
    std::cout << "  Final noise: " << noise << " (target: " << LyapunovNoiseManager::fixed_point() << ")" << std::endl;

    std::cout << "\n=== RIEMANN-FHE DEMO COMPLETE ===" << std::endl;
    return 0;
}
