with open('fhe_gravity_core.hpp', 'r') as f:
    lines = f.readlines()

# Find all lines with "attractor" and fix them
fixed = []
skip_next = False
for i, line in enumerate(lines):
    if 'double attractor =' in line and 'FIB_LADDER' in line:
        # Replace with direct value
        indent = line[:len(line) - len(line.lstrip())]
        fixed.append(f'{indent}double attractor = abs_v; // PATCHED: direct value, no Fibonacci quantization\n')
        print(f"Fixed line {i+1}")
    elif 'double attractor = abs_v' in line and i > 0:
        # Skip duplicate patches
        if 'double attractor = abs_v' in lines[i-1]:
            continue  # Skip duplicate
        fixed.append(line)
    else:
        fixed.append(line)

with open('fhe_gravity_core.hpp', 'w') as f:
    f.writelines(fixed)

print("✅ Fixed — removed duplicates")
