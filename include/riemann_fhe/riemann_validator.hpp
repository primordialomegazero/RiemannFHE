#pragma once
#include "phi_constants.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

#include <cstdio>
namespace riemann_fhe {

class RiemannValidator {
public:
    static const std::vector<double>& zeta_zeros() {
        static const std::vector<double> zeros = {
            14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
            37.586178, 40.918719, 43.327073, 48.005150, 49.773832,
            52.970321, 56.446248, 59.347044, 60.831779, 65.112544,
            67.079811, 69.546402, 72.067158, 75.704691, 77.144840,
            79.337375, 82.910381, 84.735493, 87.425275, 88.809111,
            92.491899, 94.651344, 95.870634, 98.831194, 101.317851
        };
        return zeros;
    }

    static std::vector<double> zeta_gap_ratios() {
        const auto& zeros = zeta_zeros();
        std::vector<double> gaps, ratios;
        for (size_t i = 1; i < zeros.size(); ++i)
            gaps.push_back(zeros[i] - zeros[i-1]);
        for (size_t i = 1; i < gaps.size(); ++i)
            if (gaps[i-1] > 0.001) ratios.push_back(gaps[i] / gaps[i-1]);
        return ratios;
    }

    static double zeta_phi_clustering_rate() {
        auto ratios = zeta_gap_ratios();
        size_t total = 0, near_phi = 0;
        for (double r : ratios) {
            total++;
            if (PhiConstants::is_phi_harmonic(r)) near_phi++;
        }
        return total > 0 ? static_cast<double>(near_phi) / total : 0.0;
    }

    static double validate_noise_pattern(const std::vector<double>& noise_history) {
        if (noise_history.size() < 5) return 0.0;
        std::vector<double> ratios;
        for (size_t i = 1; i < noise_history.size() - 1; ++i) {
            double g1 = noise_history[i] - noise_history[i-1];
            double g2 = noise_history[i+1] - noise_history[i];
            if (g1 > 0.001) ratios.push_back(g2 / g1);
        }
        if (ratios.empty()) return 0.0;
        size_t total = 0, near_phi = 0;
        for (double r : ratios) {
            total++;
            if (PhiConstants::is_phi_harmonic(r)) near_phi++;
        }
        double rate = total > 0 ? static_cast<double>(near_phi) / total : 0.0;
        return 1.0 - std::abs(rate - 0.654) / 0.654;
    }

    static void print_report(const std::vector<double>& noise_history) {
        double score = validate_noise_pattern(noise_history);
        printf("\n═══ RIEMANN φ-VALIDATION ═══\n");
        printf("  Zeta φ-clustering: %.1f%%\n", zeta_phi_clustering_rate() * 100);
        printf("  Validation score:  %.3f\n", score);
        printf("  Status: %s\n", score > 0.9 ? "✓ OPTIMAL" : score > 0.7 ? "≈ ACCEPTABLE" : "✗ DEGRADED");
        printf("══════════════════════════════\n");
    }
};

} // namespace riemann_fhe
