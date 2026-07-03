#pragma once
#include "phi_constants.hpp"
#include <cstdio>
#include <cmath>

namespace riemann_fhe {

class LyapunovNoiseManager {
public:
    static double lyapunov_operator(double noise) {
        double N_star = PhiConstants::FHEConstants::NOISE_FIXED_POINT;
        return noise * PhiConstants::FHEConstants::NOISE_COEFFICIENT_A
             + N_star * PhiConstants::FHEConstants::NOISE_COEFFICIENT_B;
    }

    static double fixed_point() {
        return PhiConstants::FHEConstants::NOISE_FIXED_POINT;
    }

    static void print_convergence(double initial_noise = 50.0) {
        double noise = initial_noise;
        printf("\n=== LYAPUNOV NOISE CONVERGENCE ===\n");
        printf("T(N) = N * phi^-1 + N* * (1 - phi^-1)\n\n");
        printf("k  | Noise      | Converging?\n");
        printf("---|------------|------------\n");
        for (size_t k = 1; k <= 20; ++k) {
            double prev = noise;
            noise = lyapunov_operator(noise);
            printf("%2zu | %10.6f | %s\n", k, noise, (noise < prev) ? "YES" : "NO");
        }
        printf("\nFinal: %.6f (target: %.5f)\n", noise, fixed_point());
    }
};

} // namespace riemann_fhe
