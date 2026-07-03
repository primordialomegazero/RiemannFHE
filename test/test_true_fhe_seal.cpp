#include <iostream>
#include <iomanip>
#include <cmath>
#include "../fhe/true_fhe_seal.hpp"

int main() {
    true_fhe::TrueFHE fhe;
    int pass = 0, fail = 0;
    
    std::cout << "TRUE FHE — SEAL BFV + SpiralSEAL Bootstrapping\n\n";
    
    double tests[] = {0, 1.618, 3.1416, 42, 100, -7.5, -100};
    for (double t : tests) {
        auto ct = fhe.encrypt(t);
        double dec = fhe.decrypt(ct);
        bool ok = std::abs(t - dec) < 0.01;
        if (ok) pass++; else fail++;
        std::cout << (ok ? "PASS" : "FAIL") << " enc/dec " << t << " -> " << dec << "\n";
    }
    
    auto ca = fhe.encrypt(15.0), cb = fhe.encrypt(25.0);
    auto csum = fhe.add(ca, cb);
    double r = fhe.decrypt(csum);
    bool ok = std::abs(r - 40.0) < 0.01;
    if (ok) pass++; else fail++;
    std::cout << (ok ? "PASS" : "FAIL") << " 15+25=" << r << "\n";
    
    auto cx = fhe.encrypt(6.0), cy = fhe.encrypt(7.0);
    auto cp = fhe.multiply(cx, cy);
    double rm = fhe.decrypt(cp);
    ok = std::abs(rm - 42.0) < 0.01;
    if (ok) pass++; else fail++;
    std::cout << (ok ? "PASS" : "FAIL") << " 6x7=" << rm << "\n";
    
    std::cout << "Tests: " << (pass+fail) << " | Pass: " << pass << " | Fail: " << fail << "\n";
    return fail > 0 ? 1 : 0;
}
