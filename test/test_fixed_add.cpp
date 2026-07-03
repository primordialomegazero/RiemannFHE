#include <iostream>
#include <iomanip>
#include "../include/fractal_fhe_core.hpp"
using namespace fractal_fhe;

int main() {
    FractalFHE fhe(42);
    std::cout << "═══ FIXED ADD (Direct Ciphertext) ═══\n";
    
    double pairs[][2] = {{500, 500}, {1000, 500}, {-1000, 500}};
    for (auto [a, b] : pairs) {
        auto ca = fhe.encrypt(a);
        auto cb = fhe.encrypt(b);
        auto csum = fhe.add_fixed(ca, cb);  // New fixed add
        double result = fhe.decrypt(csum);
        std::cout << "  " << a << " + " << b << " = " << std::fixed << std::setprecision(4)
                  << result << " (exp " << (a+b) << ")" << std::endl;
    }
    return 0;
}
