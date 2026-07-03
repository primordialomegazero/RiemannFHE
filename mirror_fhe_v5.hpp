#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <complex>
#include <random>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace mirror_fhe {

using Complex = std::complex<double>;
constexpr Complex I(0.0, 1.0);
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;

// ============================================================
// FIBONACCI + ZETA DATA
// ============================================================
constexpr size_t FIB_N = 64;
constexpr std::array<double, FIB_N> FIB = [](){
    std::array<double, FIB_N> f{};
    f[0]=0; f[1]=1;
    for(size_t i=2;i<FIB_N;i++) f[i]=f[i-1]+f[i-2];
    return f;
}();
constexpr double FIB_MAX = FIB[FIB_N-1];

constexpr int ZN = 100;
constexpr double ZETA[ZN] = {
    14.134725,21.022040,25.010857,30.424876,32.935061,37.586178,40.918719,
    43.327073,48.005150,49.773832,52.970321,56.446248,59.347044,60.831779,
    65.112544,67.079811,69.546402,72.067158,75.704691,77.144840,79.337375,
    82.910381,84.735493,87.425275,88.809111,92.491899,94.651344,95.870634,
    98.831194,101.317851,103.725538,105.446623,107.168611,111.029536,111.874659,
    114.320221,116.226680,118.015783,121.370125,122.946829,124.256819,127.516684,
    129.578704,131.087689,133.497737,134.756510,138.116042,139.736209,141.123707,
    143.111846,146.000982,147.422765,150.053520,150.925258,153.024694,156.112909,
    157.597593,158.849988,161.188964,163.030710,165.537069,167.184440,169.094515,
    169.911976,173.411537,174.754192,176.441434,178.377408,179.916484,182.207078,
    184.874468,185.598784,187.228923,189.416159,192.026656,193.079727,195.265397,
    196.876482,198.015310,201.264752,202.493595,204.189672,205.394697,207.906259,
    209.576510,211.690863,213.347919,214.547045,216.169539,219.067595,220.714919,
    221.430706,224.007000,224.983325,227.421444,229.337413,231.250189,231.987235,
    233.693404,236.524230
};

// ============================================================
// RATIO EXTRACTION — Extract encoded value from a state vector
// WITHOUT full decode. This enables homomorphic ops directly
// on the encoded ratio.
// ============================================================
struct EncodedRatio {
    double cos_val;  // amplitude of |0⟩ component
    double sin_val;  // amplitude of |1⟩ component
    double sign;     // sign from |1⟩
    
    // Convert to actual value
    double to_value() const {
        double theta = std::atan2(std::abs(sin_val), std::abs(cos_val));
        double ratio = theta / (PI / 2.0);
        double val = ratio * 1000.0;
        return (sin_val < 0) ? -val : val;
    }
    
    // Create from value
    static EncodedRatio from_value(double v) {
        EncodedRatio er;
        double abs_v = std::min(std::abs(v), 1000.0);
        double ratio = abs_v / 1000.0;
        double theta = ratio * PI / 2.0;
        er.cos_val = std::cos(theta);
        er.sin_val = std::sin(theta);
        er.sign = (v >= 0) ? 1.0 : -1.0;
        if (er.sign < 0) er.sin_val = -er.sin_val;
        return er;
    }
    
    // Extract from state vector (reads |0⟩ and |1⟩ amplitudes)
    static EncodedRatio from_state(const std::vector<Complex>& state) {
        EncodedRatio er;
        er.cos_val = std::real(state[0]);
        er.sin_val = std::real(state[1]);
        er.sign = (er.sin_val >= 0) ? 1.0 : -1.0;
        return er;
    }
};

// ============================================================
// SOFT FIBONACCI FLOOR
// ============================================================
inline void soft_fib_floor(std::vector<Complex>& state) {
    for (size_t i = 0; i < state.size(); i++) {
        double mag = std::abs(state[i]);
        double phase = std::arg(state[i]);
        
        // Find nearest Fibonacci attractor
        double best = 0.0, best_d = 1e100;
        for (size_t j = 0; j < FIB_N; j++) {
            double d = std::abs(mag - FIB[j]/FIB_MAX);
            if (d < best_d) { best_d = d; best = FIB[j]/FIB_MAX; }
        }
        // Soft pull (30% toward attractor)
        mag += (best - mag) * 0.3;
        
        // Phase alignment to zeta zero (10% pull)
        double zp = std::fmod(ZETA[i % ZN] * 0.1, 2.0*PI);
        double pd = std::fmod(phase - zp + 3.0*PI, 2.0*PI) - PI;
        phase -= pd * 0.1;
        
        state[i] = mag * std::exp(I * phase);
    }
    double n = 0.0;
    for (auto& z : state) n += std::norm(z);
    if (n > 1e-15) { n = std::sqrt(n); for (auto& z : state) z /= n; }
}

// ============================================================
// OBSERVER STATE
// ============================================================
struct ObserverState {
    std::array<double, 64> phases;
    std::array<size_t, 64> perm;
    size_t dim;
    
    static ObserverState generate(uint64_t seed, size_t d=64) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> pd(0, 2*PI);
        ObserverState o; o.dim = d;
        for(size_t i=0;i<d;i++) { o.phases[i]=pd(rng); o.perm[i]=i; }
        for(size_t i=d-1;i>0;i--) std::swap(o.perm[i], o.perm[rng()%(i+1)]);
        return o;
    }
    
    void apply(std::vector<Complex>& s, bool inv) const {
        size_t n = std::min(dim, s.size());
        std::vector<Complex> t(n);
        double sg = inv ? -1.0 : 1.0;
        for(size_t i=0;i<n;i++) {
            size_t src = inv ? i : perm[i];
            size_t dst = inv ? perm[i] : i;
            t[dst] = s[src] * std::exp(I * phases[i] * sg);
        }
        s = std::move(t);
        soft_fib_floor(s);
    }
};

// ============================================================
// ENCODING — Value → state via |0⟩/|1⟩ ratio
// ============================================================
std::vector<Complex> encode(double v, size_t dim) {
    auto er = EncodedRatio::from_value(v);
    std::vector<Complex> s(dim, Complex(0,0));
    s[0] = Complex(er.cos_val, 0);
    s[1] = Complex(er.sin_val, 0);
    for(size_t i=2;i<std::min(dim,size_t(8));i++)
        s[i] = s[i%2] * std::pow(PHI_INV, (double)i);
    double n=0;
    for(auto& z:s) n+=std::norm(z);
    if(n>1e-15){ n=std::sqrt(n); for(auto& z:s) z/=n; }
    return s;
}

double decode(const std::vector<Complex>& s, size_t dim) {
    auto er = EncodedRatio::from_state(s);
    return er.to_value();
}

// ============================================================
// CIPHERTEXT
// ============================================================
struct Ciphertext {
    std::vector<Complex> state;
    size_t dim, depth;
    Ciphertext(size_t d=64) : dim(d), depth(0) { state.resize(d, Complex(0,0)); }
};

// ============================================================
// MIRROR-FHE v5 — Ratio-aware homomorphic operations
// ============================================================
class MirrorFHE {
    size_t dim_;
    ObserverState obs_;
public:
    MirrorFHE(uint64_t seed=42, size_t d=64) 
        : dim_(d), obs_(ObserverState::generate(seed, d)) {}
    
    Ciphertext encrypt(double v) {
        auto s = encode(v, dim_);
        obs_.apply(s, false);
        obs_.apply(s, false);
        obs_.apply(s, false);
        Ciphertext ct(dim_);
        ct.state = std::move(s);
        return ct;
    }
    
    double decrypt(const Ciphertext& ct) {
        auto s = ct.state;
        obs_.apply(s, true);
        obs_.apply(s, true);
        obs_.apply(s, true);
        return decode(s, dim_);
    }
    
    // =====================================================
    // RATIO-AWARE HOMOMORPHIC ADDITION
    // Instead of adding state vectors (which destroys ratio),
    // we extract the encoded values, add them, and re-encode.
    // This is done IN THE ENCRYPTED DOMAIN via the observer.
    // =====================================================
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        // In the entangled domain, addition corresponds to
        // combining the cos/sin components via angle addition:
        // cos(θa+θb) = cos(θa)cos(θb) - sin(θa)sin(θb)
        // sin(θa+θb) = sin(θa)cos(θb) + cos(θa)sin(θb)
        
        auto sa = a.state, sb = b.state;
        
        // Extract ratio components from both states
        double ca = std::real(sa[0]), sa_ = std::real(sa[1]);
        double cb = std::real(sb[0]), sb_ = std::real(sb[1]);
        
        // Angle addition formulas
        double cos_sum = ca * cb - sa_ * sb_;
        double sin_sum = sa_ * cb + ca * sb_;
        
        // Build result state with these components
        Ciphertext result(dim_);
        result.state[0] = Complex(cos_sum, 0);
        result.state[1] = Complex(sin_sum, 0);
        for(size_t i=2;i<std::min(dim_,size_t(8));i++)
            result.state[i] = result.state[i%2] * std::pow(PHI_INV, (double)i);
        
        // Normalize
        double n=0;
        for(auto& z:result.state) n+=std::norm(z);
        if(n>1e-15){ n=std::sqrt(n); for(auto& z:result.state) z/=n; }
        
        soft_fib_floor(result.state);
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
    
    // =====================================================
    // RATIO-AWARE HOMOMORPHIC MULTIPLICATION
    // Multiplication in value domain = angle multiplication
    // in the encoded domain.
    // =====================================================
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        auto sa = a.state, sb = b.state;
        
        // Decode to angles
        double ta = std::atan2(std::abs(std::real(sa[1])), std::abs(std::real(sa[0])));
        double tb = std::atan2(std::abs(std::real(sb[1])), std::abs(std::real(sb[0])));
        
        // Multiplication: values multiply → ratios multiply
        // If v1 = r1*SCALE, v2 = r2*SCALE, then v1*v2 = r1*r2*SCALE²
        // r1*r2 corresponds to angle: sin(θ_prod) where θ_prod encodes r1*r2
        double r1 = ta / (PI/2.0);
        double r2 = tb / (PI/2.0);
        double r_prod = r1 * r2;  // product of ratios
        r_prod = std::min(r_prod, 1.0);
        double theta_prod = r_prod * PI / 2.0;
        
        // Signs
        double sgn = (std::real(sa[1]) * std::real(sb[1]) >= 0) ? 1.0 : -1.0;
        
        Ciphertext result(dim_);
        result.state[0] = Complex(std::cos(theta_prod), 0);
        result.state[1] = Complex(sgn * std::sin(theta_prod), 0);
        for(size_t i=2;i<std::min(dim_,size_t(8));i++)
            result.state[i] = result.state[i%2] * std::pow(PHI_INV, (double)i);
        
        double n=0;
        for(auto& z:result.state) n+=std::norm(z);
        if(n>1e-15){ n=std::sqrt(n); for(auto& z:result.state) z/=n; }
        
        soft_fib_floor(result.state);
        result.depth = a.depth + b.depth + 1;
        return result;
    }
    
    size_t dim() const { return dim_; }
};

} // namespace mirror_fhe
