#include <iostream>
#include "../pqcrypto/ml_dsa_phi/phi_poly_ml_dsa.hpp"
using namespace ml_dsa_phi;

int main() {
    auto kp = KeyPair::generate(42);
    
    PolyPhi msg;
    for (int i = 0; i < N; i++) msg.coeffs[i] = (i * 42) % Q;
    
    auto sig = sign(msg, kp, 12345);
    
    // Manual verify
    PolyPhi A;
    for (int i = 0; i < N; i++) A.coeffs[i] = (i * 1234567) % Q;
    
    PolyPhi Az_poly; Az_poly.coeffs = sig.z; Az_poly.depth = 1;
    PolyPhi Az = PolyPhi::multiply(A, Az_poly);
    PolyPhi ct = PolyPhi::multiply(sig.c, kp.public_key);
    
    // Reconstruct w'
    PolyPhi w_prime;
    for (int i = 0; i < N; i++) {
        w_prime.coeffs[i] = (Az.coeffs[i] - ct.coeffs[i]) % Q;
        if (w_prime.coeffs[i] < 0) w_prime.coeffs[i] += Q;
    }
    
    // Reconstruct c' = w' + msg
    PolyPhi c_prime = w_prime;
    for (int i = 0; i < N; i++) c_prime.coeffs[i] = (c_prime.coeffs[i] + msg.coeffs[i]) % Q;
    
    // Compare sig.c vs c_prime
    std::cout << "i\tsig.c\tc_prime\tequal?\n";
    bool all_ok = true;
    for (int i = 0; i < 5; i++) {
        bool eq = (sig.c.coeffs[i] == c_prime.coeffs[i]);
        if (!eq) all_ok = false;
        std::cout << i << "\t" << sig.c.coeffs[i] << "\t" << c_prime.coeffs[i] 
                  << "\t" << (eq ? "YES" : "NO") << "\n";
    }
    std::cout << "All match: " << (all_ok ? "YES" : "NO") << "\n";
    
    return 0;
}
