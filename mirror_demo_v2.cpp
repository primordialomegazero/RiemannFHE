#include "mirror_fhe_v2.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
using namespace mirror_fhe;

int main() {
    std::cout << "=== MIRROR-FHE v2 — Direct Amplitude Encoding ===\n\n";
    
    MirrorFHE fhe(42, 64);
    
    double tests[] = {1.618034, 3.141593, 42.0, -7.5, 0.0, 100.0, 0.618034};
    const char* names[] = {"φ", "π", "42", "-7.5", "0", "100", "φ⁻¹"};
    
    std::cout << "ENCRYPT/DECRYPT:\n";
    std::cout << "  Value | Decrypted | Error\n";
    std::cout << "  ------|-----------|------\n";
    
    for (int i = 0; i < 7; i++) {
        auto ct = fhe.encrypt(tests[i]);
        double dec = fhe.decrypt(ct);
        printf("  %6.4f | %9.4f | %.2e\n", tests[i], dec, std::abs(tests[i] - dec));
    }
    
    std::cout << "\nHOMOMORPHIC ADD: ";
    auto a = fhe.encrypt(15.5);
    auto b = fhe.encrypt(27.3);
    auto sum = fhe.add(a, b);
    printf("15.5 + 27.3 = %.2f (expected 42.8)\n", fhe.decrypt(sum));
    
    std::cout << "HOMOMORPHIC MUL: ";
    auto x = fhe.encrypt(6.0);
    auto y = fhe.encrypt(7.0);
    auto prod = fhe.multiply(x, y);
    printf("6 × 7 = %.2f (expected 42)\n", fhe.decrypt(prod));
    
    return 0;
}
