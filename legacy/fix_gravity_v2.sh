#!/bin/bash
# Fix: Bypass Fibonacci ladder, use direct φ encoding
# Replace line 286-288 with direct φ-harmonic interpolation

# Create patched version
sed -i '286,288s|.*|// PATCHED: Direct φ-encoding instead of Fibonacci ladder\n        double attractor = abs_v; // Direct value, no quantization|' fhe_gravity_core.hpp 2>/dev/null

echo "Checking if patch applied..."
grep -n "Direct φ-encoding\|attractor = abs_v" fhe_gravity_core.hpp
