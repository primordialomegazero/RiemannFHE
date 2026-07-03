#!/bin/bash
# Fix: Reduce Morse potential strength
# D_e = G_PHI * SCALE → D_e = 0.001 * SCALE (weak gravitational pull)

sed -i 's|double D_e = G_PHI \* SCALE; // well depth|double D_e = 0.001 * SCALE; // well depth (reduced for precision)|' fhe_gravity_core.hpp
sed -i 's|double D_e = G_PHI \* SCALE;|double D_e = 0.001 * SCALE; // reduced|' fhe_gravity_core.hpp

echo "✅ Patched D_e from G_PHI*SCALE to 0.001*SCALE"
