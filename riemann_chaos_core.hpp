#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <random>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

namespace riemann_chaos {

constexpr double PHI       = 1.6180339887498948482;
constexpr double PHI_INV   = 0.6180339887498948482;
constexpr double PHI_SQ    = 2.6180339887498948482;
constexpr double PHI_INV_SQ = 0.3819660112501051518;
constexpr double PI        = 3.14159265358979323846;

constexpr int ZETA_ZERO_COUNT = 100;
constexpr double ZETA_ZEROS[ZETA_ZERO_COUNT] = {
    14.134725141734693790, 21.022039638771554993, 25.010857580145688763,
    30.424876125859513210, 32.935061587739189691, 37.586178158825671257,
    40.918719012147495187, 43.327073280914999519, 48.005150881167159728,
    49.773832477672302182, 52.970321477714460644, 56.446247697063394805,
    59.347044002602353080, 60.831778524609809844, 65.112544048081606661,
    67.079810529494173714, 69.546401711173979252, 72.067157674481907582,
    75.704690699083933168, 77.144840068874805372, 79.337375020249367923,
    82.910380854086030183, 84.735492980517050106, 87.425274613125229407,
    88.809111207634465065, 92.491899270558484311, 94.651344011519545802,
    95.870634228245309274, 98.831194218193692235, 101.31785100573139123,
    103.72553804047833903, 105.44662305232609285, 107.16861118427640737,
    111.02953554316967441, 111.87465917699263716, 114.32022091545271719,
    116.22668032098252955, 118.01578296249839475, 121.37012500242023515,
    122.94682929355231447, 124.25681855434572246, 127.51668387959610604,
    129.57870419994934969, 131.08768853093230730, 133.49773720299755228,
    134.75650975337086767, 138.11604205453330843, 139.73620895212139093,
    141.12370740402178824, 143.11184580788563201, 146.00098248676520269,
    147.42276534255863735, 150.05352042078511810, 150.92525761224109135,
    153.02469381119838085, 156.11290929453298780, 157.59759281793305321,
    158.84998817142921174, 161.18896413458873780, 163.03070968718194923,
    165.53706918790005765, 167.18443997817483587, 169.09451541556892228,
    169.91197647941146924, 173.41153651959176460, 174.75419152336542338,
    176.44143429772085303, 178.37740777609980335, 179.91648402025696970,
    182.20707848436646196, 184.87446784838532174, 185.59878367770747785,
    187.22892258350117657, 189.41615865601693765, 192.02665636071378617,
    193.07972659384579948, 195.26539667952905141, 196.87648184095831694,
    198.01530967625162638, 201.26475194370378856, 202.49359451426764322,
    204.18967180310444031, 205.39469720216328604, 207.90625888780620957,
    209.57650971685619491, 211.69086259536530771, 213.34791935971266629,
    214.54704478349142195, 216.16953850826338823, 219.06759534998149999,
    220.71491883931400365, 221.43070555469333873, 224.00700025460433519,
    224.98332466958382677, 227.42144427967906923, 229.33741330552534806,
    231.25018870049916423, 231.98723525318051546, 233.69340417890830064,
    236.52422966581654941
};

constexpr int GAP_COUNT = ZETA_ZERO_COUNT - 2;
constexpr std::array<double, GAP_COUNT> ZETA_GAP_RATIOS = [](){
    std::array<double, GAP_COUNT> ratios{};
    for (int i = 0; i < GAP_COUNT; i++) {
        double g1 = ZETA_ZEROS[i+1] - ZETA_ZEROS[i];
        double g2 = ZETA_ZEROS[i+2] - ZETA_ZEROS[i+1];
        ratios[i] = (g1 > 0.001) ? g2 / g1 : 1.0;
    }
    return ratios;
}();

class RiemannChaosEngine {
private:
    double x_, y_, z_, w_;
    double phase_;
    size_t gap_idx_;
    std::array<double, 32> pool_;
    size_t pool_idx_;
    uint64_t counter_;

    static double mix64(uint64_t& x) {
        x ^= x >> 33;
        x *= 0xFF51AFD7ED558CCDULL;
        x ^= x >> 33;
        x *= 0xC4CEB9FE1A85EC53ULL;
        x ^= x >> 33;
        return static_cast<double>(x) / static_cast<double>(UINT64_MAX);
    }

public:
    explicit RiemannChaosEngine(uint64_t seed = 0) 
        : phase_(0.0), gap_idx_(0), pool_idx_(0), counter_(0) {
        if (seed == 0) { std::random_device rd; seed = rd(); }
        seed ^= 0x9E3779B97F4A7C15ULL;
        x_ = mix64(seed);
        seed += PHI * 1e15;
        y_ = mix64(seed);
        seed += PHI_SQ * 1e15;
        z_ = mix64(seed);
        seed += PI * 1e15;
        w_ = mix64(seed);
        for (int i = 0; i < 1000; i++) step();
    }

    void step() {
        double zeta = ZETA_GAP_RATIOS[gap_idx_ % GAP_COUNT];
        double perturb = std::sin(2.0 * PI * zeta * phase_);
        phase_ += zeta * 0.01;
        if (phase_ > 1000.0) phase_ -= 1000.0;
        gap_idx_++;

        double r = 3.57 + 0.43 * (0.5 + 0.5 * std::sin(phase_ * PHI_INV));
        x_ = r * x_ * (1.0 - x_);
        x_ = x_ - std::floor(x_);

        double ay = PHI_INV * y_ + 0.1 * perturb * std::sin(2.0 * PI * x_);
        y_ = ay - std::floor(ay);
        double az = PHI_INV_SQ * z_ + 0.1 * perturb * std::cos(2.0 * PI * y_);
        z_ = az - std::floor(az);
        double aw = 0.5 * w_ + 0.1 * perturb * std::sin(PI * z_);
        w_ = aw - std::floor(aw);

        double combined = x_ * PHI_INV + y_ * PHI_INV_SQ + z_ * 0.2 + w_ * 0.1;
        combined = combined - std::floor(combined);
        pool_[pool_idx_ % 32] = combined;
        pool_idx_++;
        counter_++;
    }

    double next_double() {
        for (int i = 0; i < 3; i++) step();
        double result = 0.0, wsum = 0.0;
        for (int i = 0; i < 16; i++) {
            double wt = std::pow(PHI_INV, i + 1);
            result += pool_[(pool_idx_ + i) % 32] * wt;
            wsum += wt;
        }
        result /= wsum;
        return result - std::floor(result);
    }

    uint64_t next_uint64() {
        uint64_t result = 0;
        for (int i = 0; i < 8; i++) {
            result = (result << 8) | static_cast<uint8_t>(next_double() * 256.0);
        }
        return result;
    }

    void generate(uint8_t* out, size_t count) {
        for (size_t i = 0; i < count; i++)
            out[i] = static_cast<uint8_t>(next_double() * 256.0);
    }

    double phi_clustering_rate(size_t samples = 100000) {
        std::vector<double> vals(samples);
        for (size_t i = 0; i < samples; i++) vals[i] = next_double();
        std::vector<double> ratios;
        for (size_t i = 2; i < vals.size(); i++) {
            double g1 = vals[i-1] - vals[i-2];
            double g2 = vals[i] - vals[i-1];
            if (std::abs(g1) > 1e-10) ratios.push_back(g2 / g1);
        }
        size_t cnt = 0;
        for (double r : ratios) {
            double ar = std::abs(r);
            if (std::abs(ar - PHI_INV) < 0.3 || std::abs(ar - PHI) < 0.3) cnt++;
        }
        return ratios.empty() ? 0.0 : static_cast<double>(cnt) / ratios.size();
    }
};

} // namespace riemann_chaos
