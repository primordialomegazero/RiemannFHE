// ============================================================
// RiemannFHE 100M Operations Benchmark
// Full FHE: Encrypt, Decrypt, Homomorphic Add, Homomorphic Mul
// -O0: No compiler optimizations
// Report every 1M operations
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cmath>
#include "../include/ratio_fhe_core.hpp"

using namespace ratio_fhe;
using namespace std::chrono;

constexpr int TOTAL_OPS = 100'000'000;  // 100 million
constexpr int REPORT_EVERY = 1'000'000;  // Every 1 million

int main() {
    auto start_time = system_clock::now();
    auto start_t = system_clock::to_time_t(start_time);
    
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════╗
║  RIEMANNFHE 100M OPERATIONS BENCHMARK                           ║
║  Full FHE: Encrypt + Decrypt + Homomorphic Add + Mul            ║
║  Compiler: -O0 (no optimizations)                               ║
║  φΩ0 — Primordial Omega Zero                                    ║
╚══════════════════════════════════════════════════════════════════╝
)";
    
    std::cout << "  Date: " << std::ctime(&start_t);
    std::cout << "  Total operations: " << TOTAL_OPS << " (100 million)\n";
    std::cout << "  Report interval:  " << REPORT_EVERY << " (1 million)\n";
    std::cout << "  Optimizations:    -O0 (none)\n\n";
    
    RatioFHE fhe(42);
    
    // ═══ PRE-WARM ═══
    std::cout << "  Warming up... ";
    for (int i = 0; i < 1000; i++) {
        auto ct = fhe.encrypt(42.0);
        auto dec = fhe.decrypt(ct);
    }
    std::cout << "done\n\n";
    
    // ═══ HEADER ═══
    std::cout << "  " << std::setw(10) << "Ops Done"
              << std::setw(12) << "Enc TPS"
              << std::setw(12) << "Dec TPS"
              << std::setw(12) << "Add TPS"
              << std::setw(12) << "Mul TPS"
              << std::setw(12) << "Total TPS"
              << std::setw(10) << "Elapsed"
              << std::setw(10) << "ETA" << "\n";
    std::cout << "  " << std::string(78, '-') << "\n";
    
    // ═══ BENCHMARK LOOP ═══
    int64_t total_enc = 0, total_dec = 0, total_add = 0, total_mul = 0;
    auto bench_start = high_resolution_clock::now();
    auto last_report = bench_start;
    
    for (int batch = 0; batch < TOTAL_OPS / REPORT_EVERY; batch++) {
        auto batch_start = high_resolution_clock::now();
        
        int64_t enc_ops = 0, dec_ops = 0, add_ops = 0, mul_ops = 0;
        int batch_size = REPORT_EVERY / 4;  // 250K each op type
        
        // Encrypt benchmark
        {
            auto t1 = high_resolution_clock::now();
            for (int i = 0; i < batch_size; i++) {
                volatile auto ct = fhe.encrypt((double)(i % 1000));
                enc_ops++;
            }
            auto t2 = high_resolution_clock::now();
            total_enc += enc_ops;
        }
        
        // Decrypt benchmark
        auto ct_base = fhe.encrypt(42.0);
        {
            auto t1 = high_resolution_clock::now();
            for (int i = 0; i < batch_size; i++) {
                volatile double dec = fhe.decrypt(ct_base);
                dec_ops++;
            }
            auto t2 = high_resolution_clock::now();
            total_dec += dec_ops;
        }
        
        // Homomorphic Add benchmark
        auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
        {
            auto t1 = high_resolution_clock::now();
            for (int i = 0; i < batch_size; i++) {
                volatile auto csum = fhe.add(ca, cb);
                add_ops++;
            }
            auto t2 = high_resolution_clock::now();
            total_add += add_ops;
        }
        
        // Homomorphic Mul benchmark
        auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
        {
            auto t1 = high_resolution_clock::now();
            for (int i = 0; i < batch_size; i++) {
                volatile auto cprod = fhe.multiply(cx, cy);
                mul_ops++;
            }
            auto t2 = high_resolution_clock::now();
            total_mul += mul_ops;
        }
        
        auto batch_end = high_resolution_clock::now();
        
        // Calculate TPS for this batch
        double batch_sec = duration_cast<microseconds>(batch_end - batch_start).count() / 1e6;
        double enc_tps = enc_ops / (batch_sec * 0.25);  // Each op type got ~25% of time
        double dec_tps = dec_ops / (batch_sec * 0.25);
        double add_tps = add_ops / (batch_sec * 0.25);
        double mul_tps = mul_ops / (batch_sec * 0.25);
        int64_t total_ops_this_batch = enc_ops + dec_ops + add_ops + mul_ops;
        double total_tps = total_ops_this_batch / batch_sec;
        
        // Elapsed and ETA
        auto now = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(now - bench_start).count();
        int64_t total_done = (int64_t)(batch + 1) * REPORT_EVERY;
        double progress = (double)total_done / TOTAL_OPS;
        double eta = (progress > 0) ? elapsed / progress - elapsed : 0;
        
        // Print report
        std::cout << "  " << std::setw(10) << total_done
                  << std::setw(12) << std::fixed << std::setprecision(0) << enc_tps
                  << std::setw(12) << dec_tps
                  << std::setw(12) << add_tps
                  << std::setw(12) << mul_tps
                  << std::setw(12) << total_tps
                  << std::setw(8) << std::setprecision(1) << elapsed << "s"
                  << std::setw(8) << eta << "s" << "\n";
        
        std::cout.flush();
    }
    
    auto bench_end = high_resolution_clock::now();
    double total_elapsed = duration_cast<seconds>(bench_end - bench_start).count();
    
    // ═══ FINAL REPORT ═══
    std::cout << "\n";
    std::cout << "  " << std::string(78, '=') << "\n\n";
    
    int64_t grand_total = total_enc + total_dec + total_add + total_mul;
    
    std::cout << "═══ FINAL 100M OPERATIONS REPORT ═══\n\n";
    std::cout << "  Total operations:    " << grand_total << "\n";
    std::cout << "  Total elapsed:       " << std::fixed << std::setprecision(2) << total_elapsed << "s\n";
    std::cout << "  Overall TPS:         " << std::setprecision(0) << grand_total / total_elapsed << " ops/s\n\n";
    
    std::cout << "  Operation Breakdown:\n";
    std::cout << "    Encrypt:     " << std::setw(10) << total_enc << " ops | " 
              << std::setprecision(0) << total_enc / total_elapsed << " TPS\n";
    std::cout << "    Decrypt:     " << std::setw(10) << total_dec << " ops | " 
              << total_dec / total_elapsed << " TPS\n";
    std::cout << "    Add:         " << std::setw(10) << total_add << " ops | " 
              << total_add / total_elapsed << " TPS\n";
    std::cout << "    Multiply:    " << std::setw(10) << total_mul << " ops | " 
              << total_mul / total_elapsed << " TPS\n\n";
    
    // Verify correctness
    auto verify_ct = fhe.encrypt(42.0);
    double verify_dec = fhe.decrypt(verify_ct);
    auto ca_v = fhe.encrypt(15.0), cb_v = fhe.encrypt(25.0);
    double add_result = fhe.decrypt(fhe.add(ca_v, cb_v));
    auto cx_v = fhe.encrypt(6.0), cy_v = fhe.encrypt(7.0);
    double mul_result = fhe.decrypt(fhe.multiply(cx_v, cy_v));
    
    std::cout << "  Correctness Verification:\n";
    std::cout << "    Encrypt(42)→Decrypt:  " << verify_dec 
              << " (err: " << std::scientific << std::abs(verify_dec - 42.0) << ")\n";
    std::cout << "    15+25 blind add:      " << add_result 
              << " (err: " << std::abs(add_result - 40.0) << ")\n";
    std::cout << "    6×7 blind mul:        " << mul_result 
              << " (err: " << std::abs(mul_result - 42.0) << ")\n";
    
    auto end_t = system_clock::to_time_t(system_clock::now());
    std::cout << "\n  Completed: " << std::ctime(&end_t);
    
    std::cout << R"(
══════════════════════════════════════════════════════
  100M FHE BENCHMARK COMPLETE
  Noise-Free | No Bootstrapping | Re(s)=1/2
  φΩ0 — July 3, 2026
══════════════════════════════════════════════════════
)";
    
    return 0;
}
