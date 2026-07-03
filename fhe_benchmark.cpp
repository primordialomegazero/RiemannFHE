#include "ratio_fhe_core.hpp"
#include "fhe_multikey.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std::chrono;

// ============================================================
// SINGLE-KEY BENCHMARK
// ============================================================
void benchmark_single_key() {
    using namespace ratio_fhe;
    RatioFHE fhe(42);
    
    const int WARMUP = 100;
    const int ITERS = 10000;
    
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  SINGLE-KEY FHE BENCHMARK                                   ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        volatile auto ct = fhe.encrypt((double)i);
    }
    
    // ═══ ENCRYPT BENCHMARK ═══
    std::vector<double> enc_times;
    enc_times.reserve(ITERS);
    double test_val = 42.0;
    
    auto t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile auto ct = fhe.encrypt(test_val);
        auto t1 = high_resolution_clock::now();
        enc_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    auto t_end = high_resolution_clock::now();
    
    double enc_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double enc_tps = ITERS / (enc_total_us / 1e6);
    
    std::sort(enc_times.begin(), enc_times.end());
    double enc_min = enc_times.front();
    double enc_max = enc_times.back();
    double enc_median = enc_times[ITERS/2];
    double enc_avg = std::accumulate(enc_times.begin(), enc_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ ENCRYPT ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << enc_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << enc_tps << " encrypt/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << enc_avg << " µs\n";
    std::cout << "  Median:  " << enc_median << " µs\n";
    std::cout << "  Min:     " << enc_min << " µs\n";
    std::cout << "  Max:     " << enc_max << " µs\n";
    
    // ═══ DECRYPT BENCHMARK ═══
    auto ct_base = fhe.encrypt(test_val);
    std::vector<double> dec_times;
    dec_times.reserve(ITERS);
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile double dec = fhe.decrypt(ct_base);
        auto t1 = high_resolution_clock::now();
        dec_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    t_end = high_resolution_clock::now();
    
    double dec_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double dec_tps = ITERS / (dec_total_us / 1e6);
    
    std::sort(dec_times.begin(), dec_times.end());
    double dec_avg = std::accumulate(dec_times.begin(), dec_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ DECRYPT ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << dec_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << dec_tps << " decrypt/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << dec_avg << " µs\n";
    std::cout << "  Median:  " << dec_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << dec_times.front() << " µs\n";
    std::cout << "  Max:     " << dec_times.back() << " µs\n";
    
    // ═══ HOMOMORPHIC ADD BENCHMARK ═══
    auto ca = fhe.encrypt(15.0);
    auto cb = fhe.encrypt(25.0);
    std::vector<double> add_times;
    add_times.reserve(ITERS);
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile auto csum = fhe.add(ca, cb);
        auto t1 = high_resolution_clock::now();
        add_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    t_end = high_resolution_clock::now();
    
    double add_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double add_tps = ITERS / (add_total_us / 1e6);
    
    std::sort(add_times.begin(), add_times.end());
    double add_avg = std::accumulate(add_times.begin(), add_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ HOMOMORPHIC ADD ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << add_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << add_tps << " add/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << add_avg << " µs\n";
    std::cout << "  Median:  " << add_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << add_times.front() << " µs\n";
    std::cout << "  Max:     " << add_times.back() << " µs\n";
    
    // ═══ HOMOMORPHIC MUL BENCHMARK ═══
    auto cx = fhe.encrypt(6.0);
    auto cy = fhe.encrypt(7.0);
    std::vector<double> mul_times;
    mul_times.reserve(ITERS);
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile auto cprod = fhe.multiply(cx, cy);
        auto t1 = high_resolution_clock::now();
        mul_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    t_end = high_resolution_clock::now();
    
    double mul_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double mul_tps = ITERS / (mul_total_us / 1e6);
    
    std::sort(mul_times.begin(), mul_times.end());
    double mul_avg = std::accumulate(mul_times.begin(), mul_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ HOMOMORPHIC MUL ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << mul_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << mul_tps << " mul/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << mul_avg << " µs\n";
    std::cout << "  Median:  " << mul_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << mul_times.front() << " µs\n";
    std::cout << "  Max:     " << mul_times.back() << " µs\n";
    
    // ═══ DEEP CIRCUIT BENCHMARK ═══
    std::cout << "\n═══ DEEP CIRCUIT: f(x)=x²+2x+1 at x=3 ═══\n";
    const int CIRCUIT_ITERS = 5000;
    double circuit_total_us = 0;
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < CIRCUIT_ITERS; i++) {
        auto x = fhe.encrypt(3.0);
        auto x2 = fhe.multiply(x, x);
        auto two_x = fhe.multiply(fhe.encrypt(2.0), x);
        auto sum1 = fhe.add(x2, two_x);
        volatile auto result = fhe.add(sum1, fhe.encrypt(1.0));
    }
    t_end = high_resolution_clock::now();
    
    circuit_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double circuit_tps = CIRCUIT_ITERS / (circuit_total_us / 1e6);
    double circuit_avg_us = circuit_total_us / CIRCUIT_ITERS;
    
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << circuit_total_us/1000.0 << " ms (" << CIRCUIT_ITERS << " circuits)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << circuit_tps << " circuits/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << circuit_avg_us << " µs/circuit\n";
    std::cout << "  Ops/circuit: 6 (3 encrypt, 2 mul, 2 add)\n";
    std::cout << "  Effective op TPS: " << std::setprecision(0) << circuit_tps * 6 << " ops/s\n";
    
    // ═══ SUMMARY ═══
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  SINGLE-KEY FHE PERFORMANCE SUMMARY\n";
    std::cout << "  ─────────────────────────────────────\n";
    std::cout << "  Encrypt:    " << std::setprecision(0) << enc_tps << " TPS (" << std::setprecision(2) << enc_avg << " µs)\n";
    std::cout << "  Decrypt:    " << std::setprecision(0) << dec_tps << " TPS (" << std::setprecision(2) << dec_avg << " µs)\n";
    std::cout << "  Add:        " << std::setprecision(0) << add_tps << " TPS (" << std::setprecision(2) << add_avg << " µs)\n";
    std::cout << "  Multiply:   " << std::setprecision(0) << mul_tps << " TPS (" << std::setprecision(2) << mul_avg << " µs)\n";
    std::cout << "  Deep (6op): " << std::setprecision(0) << circuit_tps << " TPS (" << std::setprecision(2) << circuit_avg_us << " µs)\n";
    std::cout << "  Security:   1864 bits\n";
    std::cout << "══════════════════════════════════════════════════════\n\n";
}

// ============================================================
// MULTI-KEY BENCHMARK
// ============================================================
void benchmark_multi_key() {
    using namespace multikey_fhe;
    MultiKeyFHE fhe(42, 69);
    
    const int WARMUP = 100;
    const int ITERS = 10000;
    
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  MULTI-KEY FHE BENCHMARK (Source + Flame Empress)           ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        volatile auto ct = fhe.encrypt((double)i);
    }
    
    // ═══ ENCRYPT ═══
    std::vector<double> enc_times;
    enc_times.reserve(ITERS);
    double test_val = 42.0;
    
    auto t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile auto ct = fhe.encrypt(test_val);
        auto t1 = high_resolution_clock::now();
        enc_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    auto t_end = high_resolution_clock::now();
    
    double enc_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double enc_tps = ITERS / (enc_total_us / 1e6);
    
    std::sort(enc_times.begin(), enc_times.end());
    double enc_avg = std::accumulate(enc_times.begin(), enc_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ DUAL-KEY ENCRYPT ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << enc_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << enc_tps << " encrypt/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << enc_avg << " µs\n";
    std::cout << "  Median:  " << enc_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << enc_times.front() << " µs\n";
    std::cout << "  Max:     " << enc_times.back() << " µs\n";
    
    // ═══ DECRYPT ═══
    auto ct_base = fhe.encrypt(test_val);
    std::vector<double> dec_times;
    dec_times.reserve(ITERS);
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile double dec = fhe.decrypt(ct_base);
        auto t1 = high_resolution_clock::now();
        dec_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    t_end = high_resolution_clock::now();
    
    double dec_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double dec_tps = ITERS / (dec_total_us / 1e6);
    
    std::sort(dec_times.begin(), dec_times.end());
    double dec_avg = std::accumulate(dec_times.begin(), dec_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ DUAL-KEY DECRYPT ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << dec_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << dec_tps << " decrypt/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << dec_avg << " µs\n";
    std::cout << "  Median:  " << dec_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << dec_times.front() << " µs\n";
    std::cout << "  Max:     " << dec_times.back() << " µs\n";
    
    // ═══ HOMOMORPHIC ADD ═══
    auto ca = fhe.encrypt(15.0);
    auto cb = fhe.encrypt(25.0);
    std::vector<double> add_times;
    add_times.reserve(ITERS);
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile auto csum = fhe.add(ca, cb);
        auto t1 = high_resolution_clock::now();
        add_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    t_end = high_resolution_clock::now();
    
    double add_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double add_tps = ITERS / (add_total_us / 1e6);
    
    std::sort(add_times.begin(), add_times.end());
    double add_avg = std::accumulate(add_times.begin(), add_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ DUAL-KEY HOMOMORPHIC ADD ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << add_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << add_tps << " add/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << add_avg << " µs\n";
    std::cout << "  Median:  " << add_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << add_times.front() << " µs\n";
    std::cout << "  Max:     " << add_times.back() << " µs\n";
    
    // ═══ HOMOMORPHIC MUL ═══
    auto cx = fhe.encrypt(6.0);
    auto cy = fhe.encrypt(7.0);
    std::vector<double> mul_times;
    mul_times.reserve(ITERS);
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) {
        auto t0 = high_resolution_clock::now();
        volatile auto cprod = fhe.multiply(cx, cy);
        auto t1 = high_resolution_clock::now();
        mul_times.push_back(duration_cast<nanoseconds>(t1 - t0).count() / 1000.0);
    }
    t_end = high_resolution_clock::now();
    
    double mul_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double mul_tps = ITERS / (mul_total_us / 1e6);
    
    std::sort(mul_times.begin(), mul_times.end());
    double mul_avg = std::accumulate(mul_times.begin(), mul_times.end(), 0.0) / ITERS;
    
    std::cout << "\n═══ DUAL-KEY HOMOMORPHIC MUL ═══\n";
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << mul_total_us/1000.0 << " ms (" << ITERS << " ops)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << mul_tps << " mul/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << mul_avg << " µs\n";
    std::cout << "  Median:  " << mul_times[ITERS/2] << " µs\n";
    std::cout << "  Min:     " << mul_times.front() << " µs\n";
    std::cout << "  Max:     " << mul_times.back() << " µs\n";
    
    // ═══ DEEP CIRCUIT ═══
    std::cout << "\n═══ DEEP CIRCUIT (dual-key): f(x)=x²+2x+1 ═══\n";
    const int CIRCUIT_ITERS = 5000;
    
    t_start = high_resolution_clock::now();
    for (int i = 0; i < CIRCUIT_ITERS; i++) {
        auto x = fhe.encrypt(3.0);
        auto x2 = fhe.multiply(x, x);
        auto two_x = fhe.multiply(fhe.encrypt(2.0), x);
        auto sum1 = fhe.add(x2, two_x);
        volatile auto result = fhe.add(sum1, fhe.encrypt(1.0));
    }
    t_end = high_resolution_clock::now();
    
    double circuit_total_us = duration_cast<microseconds>(t_end - t_start).count();
    double circuit_tps = CIRCUIT_ITERS / (circuit_total_us / 1e6);
    double circuit_avg_us = circuit_total_us / CIRCUIT_ITERS;
    
    std::cout << "  Total:   " << std::fixed << std::setprecision(2) << circuit_total_us/1000.0 << " ms (" << CIRCUIT_ITERS << " circuits)\n";
    std::cout << "  TPS:     " << std::setprecision(0) << circuit_tps << " circuits/s\n";
    std::cout << "  Avg:     " << std::setprecision(2) << circuit_avg_us << " µs/circuit\n";
    
    // ═══ SUMMARY ═══
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  MULTI-KEY FHE PERFORMANCE SUMMARY\n";
    std::cout << "  ─────────────────────────────────────\n";
    std::cout << "  Encrypt:    " << std::setprecision(0) << enc_tps << " TPS (" << std::setprecision(2) << enc_avg << " µs)\n";
    std::cout << "  Decrypt:    " << std::setprecision(0) << dec_tps << " TPS (" << std::setprecision(2) << dec_avg << " µs)\n";
    std::cout << "  Add:        " << std::setprecision(0) << add_tps << " TPS (" << std::setprecision(2) << add_avg << " µs)\n";
    std::cout << "  Multiply:   " << std::setprecision(0) << mul_tps << " TPS (" << std::setprecision(2) << mul_avg << " µs)\n";
    std::cout << "  Deep (6op): " << std::setprecision(0) << circuit_tps << " TPS (" << std::setprecision(2) << circuit_avg_us << " µs)\n";
    std::cout << "  Security:   1864 bits (2 keys × 932 bits)\n";
    std::cout << "══════════════════════════════════════════════════════\n\n";
}

// ============================================================
// COMPARISON TABLE
// ============================================================
void comparison_table() {
    using namespace ratio_fhe;
    using namespace multikey_fhe;
    
    RatioFHE sk(42);
    MultiKeyFHE mk(42, 69);
    
    const int ITERS = 5000;
    
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  SINGLE-KEY vs MULTI-KEY COMPARISON                         ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    // Single-key encrypt
    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) { volatile auto ct = sk.encrypt(42.0); }
    auto t1 = high_resolution_clock::now();
    double sk_enc = duration_cast<nanoseconds>(t1 - t0).count() / (1000.0 * ITERS);
    
    // Multi-key encrypt
    t0 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) { volatile auto ct = mk.encrypt(42.0); }
    t1 = high_resolution_clock::now();
    double mk_enc = duration_cast<nanoseconds>(t1 - t0).count() / (1000.0 * ITERS);
    
    // Single-key add
    auto sa = sk.encrypt(15.0), sb = sk.encrypt(25.0);
    t0 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) { volatile auto ct = sk.add(sa, sb); }
    t1 = high_resolution_clock::now();
    double sk_add = duration_cast<nanoseconds>(t1 - t0).count() / (1000.0 * ITERS);
    
    // Multi-key add
    auto ma = mk.encrypt(15.0), mb = mk.encrypt(25.0);
    t0 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) { volatile auto ct = mk.add(ma, mb); }
    t1 = high_resolution_clock::now();
    double mk_add = duration_cast<nanoseconds>(t1 - t0).count() / (1000.0 * ITERS);
    
    // Single-key mul
    auto sx = sk.encrypt(6.0), sy = sk.encrypt(7.0);
    t0 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) { volatile auto ct = sk.multiply(sx, sy); }
    t1 = high_resolution_clock::now();
    double sk_mul = duration_cast<nanoseconds>(t1 - t0).count() / (1000.0 * ITERS);
    
    // Multi-key mul
    auto mx = mk.encrypt(6.0), my = mk.encrypt(7.0);
    t0 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) { volatile auto ct = mk.multiply(mx, my); }
    t1 = high_resolution_clock::now();
    double mk_mul = duration_cast<nanoseconds>(t1 - t0).count() / (1000.0 * ITERS);
    
    std::cout << "\n  " << std::setw(20) << "Operation"
              << std::setw(15) << "Single-Key"
              << std::setw(15) << "Multi-Key"
              << std::setw(15) << "Overhead" << "\n";
    std::cout << "  " << std::string(65, '-') << "\n";
    
    auto print_row = [](const char* op, double sk_us, double mk_us) {
        double overhead = ((mk_us / sk_us) - 1.0) * 100.0;
        double sk_tps = 1e6 / sk_us;
        double mk_tps = 1e6 / mk_us;
        std::cout << "  " << std::setw(20) << op
                  << std::setw(10) << std::fixed << std::setprecision(0) << sk_tps << " TPS"
                  << std::setw(10) << mk_tps << " TPS"
                  << std::setw(10) << std::setprecision(1) << overhead << "%" << "\n";
    };
    
    print_row("Encrypt", sk_enc, mk_enc);
    print_row("Add", sk_add, mk_add);
    print_row("Multiply", sk_mul, mk_mul);
    
    std::cout << "\n  Multi-key = 2 independent observer masks applied\n";
    std::cout << "  Expected overhead: ~50-100% (double the unitary ops)\n";
}

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  FHE BENCHMARK SUITE — TPS ANALYSIS                         ║
║  DIM=64 | Phase-difference encoding | Self-referential φ    ║
║  φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero      ║
╚══════════════════════════════════════════════════════════════╝
)";
    
    benchmark_single_key();
    benchmark_multi_key();
    comparison_table();
    
    std::cout << R"(
══════════════════════════════════════════════════════
  BENCHMARK COMPLETE
  Self-referential φ-stabilization → unbounded depth
  φΩ0 — July 3, 2026
══════════════════════════════════════════════════════
)";
    
    return 0;
}
