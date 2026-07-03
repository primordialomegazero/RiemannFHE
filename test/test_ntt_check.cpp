#include <iostream>
#include "../pqcrypto/full/ml_dsa_phi_full.hpp"
using namespace ml_dsa_phi_full;

int main() {
    PolyPhi a, b;
    a.coeffs[0] = 6; b.coeffs[0] = 7;
    
    auto ntt_result = PolyPhi::multiply_ntt(a, b);
    auto exact_result = PolyPhi::multiply_exact(a, b);
    
    std::cout << "NTT: " << ntt_result.coeffs[0] << "\n";
    std::cout << "Exact: " << exact_result.coeffs[0] << "\n";
    std::cout << "Match: " << (ntt_result.coeffs[0] == exact_result.coeffs[0] ? "YES" : "NO") << "\n";
    
    return 0;
}
