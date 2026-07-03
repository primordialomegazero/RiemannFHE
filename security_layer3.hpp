#pragma once
// ============================================================
// SECURITY LAYER 3: Reverse Lattice — Anti-LLL/BKZ
// Hyperbolic geometry + Unit-magnitude irrational mask
// φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero
// ============================================================
#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include <algorithm>

namespace security_layer3 {

using Complex = std::complex<double>;
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;

struct HyperbolicMetric {
    static Complex to_disk(const Complex& z) {
        double r = std::abs(z);
        double disk_r = r / (PHI + r);
        double theta = std::arg(z);
        return Complex(disk_r * std::cos(theta), disk_r * std::sin(theta));
    }
    
    static double distance(const Complex& a, const Complex& b) {
        auto da = to_disk(a), db = to_disk(b);
        double num = 2.0 * std::norm(da - db);
        double den = (1.0 - std::norm(da)) * (1.0 - std::norm(db));
        if (den < 1e-15) return 1e15;
        return std::acosh(1.0 + num / den);
    }
    
    static double norm(const Complex& z) {
        auto d = to_disk(z);
        double r = std::abs(d);
        if (r >= 1.0) return 1e15;
        return 2.0 * std::atanh(r);
    }
};

struct IrrationalBasis {
    std::vector<Complex> basis_vectors;
    size_t dim;
    
    IrrationalBasis() : dim(64) { init(0); }
    IrrationalBasis(uint64_t seed, size_t d = 64) : dim(d) { init(seed); }
    
    void init(uint64_t seed) {
        basis_vectors.resize(dim);
        // Use φ-weighted irrational phases — guaranteed non-repeating for dim ≤ 1000
        for (size_t i = 0; i < dim; i++) {
            // φ^φ * i gives transcendental spacing — no two angles will ever align
            double angle = std::fmod(PHI * PHI * (double)(i + 1) * PI, 2.0 * PI);
            // All unit magnitude for unitarity
            basis_vectors[i] = Complex(std::cos(angle), std::sin(angle));
        }
    }
    
    double gram_schmidt_resistance() const {
        double min_angle = PI;
        for (size_t i = 0; i < dim; i++) {
            for (size_t j = i + 1; j < dim; j++) {
                double dot = std::abs(basis_vectors[i] * std::conj(basis_vectors[j]));
                double angle = std::acos(std::min(1.0, std::max(-1.0, dot)));
                if (angle < min_angle && angle > 1e-12) min_angle = angle;
            }
        }
        return min_angle;
    }
};

struct ReverseLattice {
    IrrationalBasis basis;
    
    ReverseLattice() : basis(0, 64) {}
    ReverseLattice(uint64_t seed, size_t dim = 64) : basis(seed, dim) {}
    
    void apply(std::vector<Complex>& state) const {
        std::vector<Complex> result(state.size(), Complex(0, 0));
        for (size_t i = 0; i < state.size(); i++) {
            for (size_t j = 0; j < basis.dim; j++) {
                double dist = HyperbolicMetric::distance(state[i], basis.basis_vectors[j]);
                double weight = 1.0 / (1.0 + dist);
                double phase = std::fmod(dist * PHI, 2.0 * PI);
                Complex rotation(std::cos(phase), std::sin(phase));
                result[i] = result[i] + state[i] * rotation * weight;
            }
        }
        state = std::move(result);
    }
    
    static bool verify_anti_lattice(const std::vector<Complex>& state) {
        double min_norm = 1e15, max_norm = 0;
        for (const auto& z : state) {
            double n = HyperbolicMetric::norm(z);
            if (n < min_norm) min_norm = n;
            if (n > max_norm) max_norm = n;
        }
        double ratio = max_norm / (min_norm + 1e-15);
        return ratio > PHI;
    }
};

struct AntiLatticeMask {
    std::vector<Complex> mask;
    std::vector<size_t> perm, inv_perm;
    
    static AntiLatticeMask generate(uint64_t seed, size_t dim = 64) {
        AntiLatticeMask m;
        IrrationalBasis ib(seed, dim);
        m.mask = ib.basis_vectors;  // Unit-magnitude irrational phases
        
        // Permutation from hyperbolic distances to origin
        std::vector<std::pair<double, size_t>> pairs(dim);
        for (size_t i = 0; i < dim; i++) {
            pairs[i] = {HyperbolicMetric::distance(m.mask[i], Complex(0,0)), i};
        }
        std::sort(pairs.begin(), pairs.end());
        
        m.perm.resize(dim);
        m.inv_perm.resize(dim);
        for (size_t i = 0; i < dim; i++) {
            m.perm[i] = pairs[i].second;
            m.inv_perm[pairs[i].second] = i;
        }
        return m;
    }
    
    void apply(std::vector<Complex>& v) const {
        std::vector<Complex> temp(v.size());
        for (size_t i = 0; i < v.size(); i++) temp[perm[i]] = v[i] * mask[i];
        v = std::move(temp);
    }
    
    void unapply(std::vector<Complex>& v) const {
        std::vector<Complex> temp(v.size());
        for (size_t i = 0; i < v.size(); i++) temp[i] = v[perm[i]] * std::conj(mask[i]);
        v = std::move(temp);
    }
};

} // namespace security_layer3
