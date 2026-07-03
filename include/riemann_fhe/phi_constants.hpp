#pragma once
#include <cmath>
#include <array>
#include <concepts>

namespace riemann_fhe {

struct PhiConstants {
    static constexpr double PHI         = 1.6180339887498948482;
    static constexpr double PHI_INV     = 0.6180339887498948482;
    static constexpr double PHI_HALF    = 0.8090169943749474241;
    static constexpr double PHI_SQ      = 2.6180339887498948482;
    static constexpr double PHI_INV_SQ  = 0.3819660112501051518;
    static constexpr double PHI_CUBE    = 4.2360679774997896964;
    static constexpr double PHI_NEG4    = 0.1458980337503154555;
    
    struct RiemannMarkers {
        static constexpr double PEAK_PRIMARY   = PHI_HALF;
        static constexpr double PEAK_SECONDARY = PHI_INV;
        static constexpr double PEAK_TERTIARY  = PHI;
        static constexpr double CLUSTERING_TAU = 0.3;
        static constexpr std::array<double, 3> ALL_PEAKS = {PHI_HALF, PHI_INV, PHI};
    };
    
    struct FHEConstants {
        static constexpr double OPTIMAL_CONTRACTION = PHI_INV;
        static constexpr double NOISE_COEFFICIENT_A = PHI_INV;
        static constexpr double NOISE_COEFFICIENT_B = 1.0 - PHI_INV;
        static constexpr double NOISE_FIXED_POINT = 1.828154;
        static constexpr double MAX_SAFE_NOISE = 100.0;
        static constexpr double NOISE_FLOOR = 0.001;
    };
    
    static constexpr bool is_phi_harmonic(double value, double tolerance = 0.3) {
        for (double peak : RiemannMarkers::ALL_PEAKS)
            if (std::abs(value - peak) < tolerance) return true;
        return false;
    }
};

} // namespace riemann_fhe
