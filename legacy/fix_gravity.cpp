// Quick test: Reduce Morse potential strength
#include <iostream>
#include <cmath>
using namespace std;

int main() {
    const double SCALE = 1000.0;
    
    // Original: D_e = 1.0 → too strong, everything collapses to ±SCALE
    // Fix: D_e = 0.001 → weak pull, allows values between -SCALE and +SCALE
    
    double values[] = {0, 1.618, 3.14, 13, 21, 34, 42, 55, 89, 100, -7.5, -100};
    
    for (double v : values) {
        // Normalize
        double x = max(-1.0, min(1.0, v / SCALE));
        
        // Weak Morse potential (D_e = 0.01 instead of 1.0)
        double D_e = 0.01;
        double a = 3.0;
        double attractor = round(x * 10) / 10; // Nearest 0.1
        double dx = x - attractor;
        double V = D_e * pow(1.0 - exp(-a * abs(dx)), 2);
        
        // Apply gentle pull
        if (dx > 0) x -= V;
        else x += V;
        
        double recovered = x * SCALE;
        printf("  %8.3f → %10.4f (error: %.2f)\n", v, recovered, abs(v - recovered));
    }
    
    return 0;
}
