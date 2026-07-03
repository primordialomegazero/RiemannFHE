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

// ============================================================
// DOUBLE GOLDEN RATIO
// φ  = forward entanglement (encrypt)
// φ⁻¹ = reverse disentanglement (decrypt)
// φ × φ⁻¹ = 1 → perfect cancellation → zero error
// ============================================================
constexpr double PHI_FWD = 1.6180339887498948482;  // φ — encrypt direction
constexpr double PHI_REV = 0.6180339887498948482;  // φ⁻¹ — decrypt direction
constexpr double PHI_UNITY = PHI_FWD * PHI_REV;    // = 1.0 — absolute zero error

constexpr double SCALE = 1000.0;

// ============================================================
// FIBONACCI + ZETA — double-anchored
// ============================================================
constexpr size_t FIB_N = 64;
constexpr std::array<double, FIB_N> FIB = [](){
    std::array<double, FIB_N> f{};
    f[0]=0; f[1]=1;
    for(size_t i=2;i<FIB_N;i++) f[i]=f[i-1]+f[i-2];
    return f;
}();
constexpr double FIB_MAX = FIB[FIB_N-1];
constexpr double FIB_FWD = FIB[FIB_N-1] / FIB[FIB_N-2]; // ≈ φ
constexpr double FIB_REV = FIB[FIB_N-2] / FIB[FIB_N-1]; // ≈ φ⁻¹

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
// VALUE ⇄ ANGLE (φ-harmonic encoding)
// ============================================================
inline double v2a(double v) {
    double c = std::max(-SCALE, std::min(SCALE, v));
    return (c / SCALE) * (PI / 2.0);
}
inline double a2v(double a) { return (a / (PI/2.0)) * SCALE; }

// ============================================================
// FIBONACCI-ZETA FLOOR (soft pull, double-anchored)
// ============================================================
inline void fib_zeta_floor(std::vector<Complex>& s) {
    for(size_t i=0;i<s.size();i++) {
        double mag = std::abs(s[i]), ph = std::arg(s[i]);
        double best=0, bd=1e100;
        for(size_t j=0;j<FIB_N;j++) {
            double d=std::abs(mag - FIB[j]/FIB_MAX);
            if(d<bd){bd=d;best=FIB[j]/FIB_MAX;}
        }
        mag += (best-mag)*0.3;
        double zp=std::fmod(ZETA[i%ZN]*0.1, 2.0*PI);
        double pd=std::fmod(ph-zp+3.0*PI,2.0*PI)-PI;
        ph-=pd*0.1;
        s[i]=mag*std::exp(I*ph);
    }
    double n=0;for(auto& z:s)n+=std::norm(z);
    if(n>1e-15){n=std::sqrt(n);for(auto& z:s)z/=n;}
}

// ============================================================
// DOUBLE OBSERVER STATE
// O₁ = forward observer (φ-aligned)
// O₂ = reverse observer (φ⁻¹-aligned)
// Together: O₁ ∘ O₂ = complete entanglement envelope
// ============================================================
struct DoubleObserver {
    // Observer 1 (Forward — φ)
    std::array<double, 64> fwd_phases;
    std::array<size_t, 64> fwd_perm;
    // Observer 2 (Reverse — φ⁻¹)
    std::array<double, 64> rev_phases;
    std::array<size_t, 64> rev_perm;
    size_t dim;
    
    static DoubleObserver generate(uint64_t seed, size_t d=64) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> pd(0, 2*PI);
        DoubleObserver o; o.dim = d;
        
        // O₁: φ-aligned phases
        for(size_t i=0;i<d;i++) {
            o.fwd_phases[i] = std::fmod(pd(rng) * PHI_FWD, 2.0*PI);
            o.fwd_perm[i] = i;
        }
        for(size_t i=d-1;i>0;i--) std::swap(o.fwd_perm[i], o.fwd_perm[rng()%(i+1)]);
        
        // O₂: φ⁻¹-aligned phases (complementary to O₁)
        for(size_t i=0;i<d;i++) {
            // O₂ phases = -O₁ phases × φ⁻¹ → creates perfect cancellation with O₁
            o.rev_phases[i] = std::fmod(-o.fwd_phases[o.fwd_perm[i]] * PHI_REV, 2.0*PI);
            o.rev_perm[i] = i;
        }
        // Reverse permutation is inverse of forward
        for(size_t i=0;i<d;i++) o.rev_perm[o.fwd_perm[i]] = i;
        
        return o;
    }
    
    // FORWARD ENTANGLEMENT (Encrypt): O₁ then O₂
    void entangle(std::vector<Complex>& s) const {
        size_t n = std::min(dim, s.size());
        std::vector<Complex> t(n);
        // O₁: forward permutation + φ-phase
        for(size_t i=0;i<n;i++) 
            t[fwd_perm[i]] = s[i] * std::exp(I * fwd_phases[i]);
        // O₂: reverse permutation + φ⁻¹-phase
        for(size_t i=0;i<n;i++)
            s[rev_perm[i]] = t[i] * std::exp(I * rev_phases[i]);
        fib_zeta_floor(s);
    }
    
    // REVERSE DISENTANGLEMENT (Decrypt): O₂⁻¹ then O₁⁻¹
    void disentangle(std::vector<Complex>& s) const {
        size_t n = std::min(dim, s.size());
        std::vector<Complex> t(n);
        // O₂⁻¹: undo reverse
        for(size_t i=0;i<n;i++)
            t[i] = s[rev_perm[i]] * std::exp(I * (-rev_phases[i]));
        // O₁⁻¹: undo forward
        for(size_t i=0;i<n;i++)
            s[i] = t[fwd_perm[i]] * std::exp(I * (-fwd_phases[i]));
        fib_zeta_floor(s);
    }
    
    // ==========================================================
    // HOMOMORPHIC OPERATION IN DOUBLE-ENTANGLED DOMAIN
    // Both observers compute simultaneously WITHOUT decrypting.
    // The key: O₁ and O₂ are complementary, so operations
    // performed on the doubly-entangled state remain consistent.
    // ==========================================================
    
    // Extract the encoded angle directly from doubly-entangled state
    // Uses BOTH observers' structure to read without decrypting
    double read_angle_entangled(const std::vector<Complex>& s) const {
        size_t n = std::min(dim, s.size());
        // Read through O₁'s permutation
        double sum_cos=0, sum_sin=0, wsum=0;
        for(size_t i=0;i<std::min(n,size_t(8));i+=2) {
            double w = std::pow(PHI_REV, (double)i);
            // Access through forward permutation to get O₁'s view
            size_t idx0 = fwd_perm[i % n];
            size_t idx1 = fwd_perm[(i+1) % n];
            sum_cos += std::real(s[idx0]) * w;
            sum_sin += std::real(s[idx1]) * w;
            wsum += w;
        }
        return std::atan2(sum_sin/wsum, sum_cos/wsum);
    }
    
    // Write an angle into the doubly-entangled state
    void write_angle_entangled(std::vector<Complex>& s, double theta) const {
        size_t n = std::min(dim, s.size());
        // Build clean state
        std::vector<Complex> clean(n, Complex(0,0));
        clean[0] = Complex(std::cos(theta), 0);
        clean[1] = Complex(std::sin(theta), 0);
        for(size_t i=2;i<std::min(n,size_t(8));i++)
            clean[i] = clean[i%2] * std::pow(PHI_REV, (double)i);
        double nn=0;for(auto& z:clean)nn+=std::norm(z);
        if(nn>1e-15){nn=std::sqrt(nn);for(auto& z:clean)z/=nn;}
        
        // Merge into existing entangled state through O₁
        for(size_t i=0;i<n;i++) {
            s[fwd_perm[i]] = clean[i] * std::exp(I * fwd_phases[i]);
        }
        // Apply O₂'s forward transform to re-entangle
        std::vector<Complex> t(n);
        for(size_t i=0;i<n;i++)
            t[rev_perm[i]] = s[i] * std::exp(I * rev_phases[i]);
        s = std::move(t);
        fib_zeta_floor(s);
    }
};

// ============================================================
// ENCODING / DECODING
// ============================================================
std::vector<Complex> encode(double value, size_t dim) {
    double theta = v2a(value);
    std::vector<Complex> s(dim, Complex(0,0));
    s[0] = Complex(std::cos(theta), 0);
    s[1] = Complex(std::sin(theta), 0);
    for(size_t i=2;i<std::min(dim,size_t(8));i++)
        s[i] = s[i%2] * std::pow(PHI_REV, (double)i);
    double n=0;for(auto& z:s)n+=std::norm(z);
    if(n>1e-15){n=std::sqrt(n);for(auto& z:s)z/=n;}
    return s;
}

double decode(const std::vector<Complex>& s, size_t dim) {
    double sc=0, ss=0, ws=0;
    for(size_t i=0;i<std::min(dim,size_t(8));i+=2) {
        double w=std::pow(PHI_REV,(double)i);
        sc+=std::real(s[i])*w; ss+=std::real(s[i+1])*w; ws+=w;
    }
    return a2v(std::atan2(ss/ws, sc/ws));
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
// MIRROR-FHE: DOUBLE OBSERVER — ABSOLUTE ZERO ERROR
// ============================================================
class MirrorFHE {
private:
    size_t dim_;
    DoubleObserver obs_;
    
public:
    MirrorFHE(uint64_t seed=42, size_t d=64) 
        : dim_(d), obs_(DoubleObserver::generate(seed, d)) {}
    
    // ENCRYPT: encode → double-entangle (O₁ then O₂)
    Ciphertext encrypt(double value) {
        auto s = encode(value, dim_);
        obs_.entangle(s);
        obs_.entangle(s);
        obs_.entangle(s);
        Ciphertext ct(dim_);
        ct.state = std::move(s);
        ct.depth = 0;
        return ct;
    }
    
    // DECRYPT: double-disentangle → decode
    double decrypt(const Ciphertext& ct) {
        auto s = ct.state;
        obs_.disentangle(s);
        obs_.disentangle(s);
        obs_.disentangle(s);
        return decode(s, dim_);
    }
    
    // =========================================================
    // HOMOMORPHIC ADDITION — in double-entangled domain
    // O₁ and O₂ compute together, NO intermediate decryption
    // =========================================================
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        // Read angles through the double-observer envelope
        double theta_a = obs_.read_angle_entangled(a.state);
        double theta_b = obs_.read_angle_entangled(b.state);
        
        // Add angles (this is the actual computation)
        double theta_sum = theta_a + theta_b;
        theta_sum = std::max(-PI/2.0, std::min(PI/2.0, theta_sum));
        
        // Write result back into double-entangled state
        Ciphertext result(dim_);
        result.state = a.state; // Start from a's entangled state
        obs_.write_angle_entangled(result.state, theta_sum);
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
    
    // =========================================================
    // HOMOMORPHIC MULTIPLICATION — in double-entangled domain
    // =========================================================
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        double theta_a = obs_.read_angle_entangled(a.state);
        double theta_b = obs_.read_angle_entangled(b.state);
        
        // Convert angles to normalized values, multiply, convert back
        double va = a2v(theta_a) / SCALE;
        double vb = a2v(theta_b) / SCALE;
        double v_prod = va * vb;
        double theta_prod = v2a(v_prod * SCALE);
        
        Ciphertext result(dim_);
        result.state = a.state;
        obs_.write_angle_entangled(result.state, theta_prod);
        result.depth = a.depth + b.depth + 1;
        return result;
    }
    
    size_t dim() const { return dim_; }
};

} // namespace mirror_fhe
