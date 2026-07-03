#include <iostream>
#include "../pqcrypto/ml_dsa_phi/phi_poly_ml_dsa.hpp"
using namespace ml_dsa_phi;

int main() {
    auto kp = KeyPair::generate(42);
    
    PolyPhi msg;
    for (int i = 0; i < N; i++) msg.coeffs[i] = (i * 42) % Q;
    
    // Sign
    uint64_t nonce = 12345;
    PolyPhi y;
    for (int i = 0; i < N; i++) y.coeffs[i] = (int32_t)((int64_t)(nonce * PHI * (i + 1)) % Q);
    y.depth = 1;
    
    PolyPhi A;
    for (int i = 0; i < N; i++) A.coeffs[i] = (i * 1234567) % Q;
    
    PolyPhi w = PolyPhi::multiply(A, y);
    
    // Challenge
    PolyPhi c = w;
    for (int i = 0; i < N; i++) c.coeffs[i] = (c.coeffs[i] + msg.coeffs[i]) % Q;
    c.depth = 0;
    
    // Response
    PolyPhi cs = PolyPhi::multiply(c, kp.secret_key);
    PolyPhi z;
    for (int i = 0; i < N; i++) z.coeffs[i] = (y.coeffs[i] + cs.coeffs[i]) % Q;
    z.depth = 1;
    
    // Verify
    PolyPhi Az_poly; Az_poly.coeffs = z.coeffs; Az_poly.depth = 1;
    PolyPhi Az = PolyPhi::multiply(A, Az_poly);
    PolyPhi ct = PolyPhi::multiply(c, kp.public_key);
    
    // Check w' = Az - ct
    PolyPhi w_prime;
    for (int i = 0; i < N; i++) {
        w_prime.coeffs[i] = (Az.coeffs[i] - ct.coeffs[i]) % Q;
        if (w_prime.coeffs[i] < 0) w_prime.coeffs[i] += Q;
    }
    
    // Check first 5 coefficients
    std::cout << "i\tw\tw'\tAz\tct\tequal?\n";
    for (int i = 0; i < 5; i++) {
        bool eq = (w.coeffs[i] == w_prime.coeffs[i]);
        std::cout << i << "\t" << w.coeffs[i] << "\t" << w_prime.coeffs[i] 
                  << "\t" << Az.coeffs[i] << "\t" << ct.coeffs[i]
                  << "\t" << (eq ? "YES" : "NO") << "\n";
    }
    
    return 0;
}
