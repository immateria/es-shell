#!/usr/bin/env es
# Comprehensive test suite for negative number arithmetic
# Tests the hybrid integer/float arithmetic system with emphasis on negative numbers

echo '=== Testing Negative Number Arithmetic ==='
echo

echo '--- Basic Negative Arithmetic ---'
echo 'Testing ${-5 + 3}:'
echo ${-5 + 3}

echo 'Testing ${-10 × 2}:'  
echo ${-10 × 2}

echo 'Testing ${-8 ÷ 2}:'
echo ${-8 ÷ 2}

echo 'Testing ${-15 - 5}:'
echo ${-15 - 5}

echo 'Testing ${-7 + (-3)}:'
echo ${-7 + (-3)}

echo 'Testing ${-4 × (-6)}:'
echo ${-4 × (-6)}

echo

echo '--- Mixed Positive/Negative Operations ---'
echo 'Testing ${5 + (-3)}:'
echo ${5 + (-3)}

echo 'Testing ${(-8) × 7}:'
echo ${(-8) × 7}

echo 'Testing ${12 ÷ (-3)}:'
echo ${12 ÷ (-3)}

echo 'Testing ${(-15) - (-5)}:'
echo ${(-15) - (-5)}

echo

echo '--- Zero Handling ---'
echo 'Testing ${0 - 0} (negative zero):'  
echo ${0 - 0}

echo 'Testing ${0 + (-5)}:'
echo ${0 + (-5)}

echo 'Testing ${(-7) × 0}:'
echo ${(-7) × 0}

echo 'Testing ${0 ÷ (-1)}:'
echo ${0 ÷ (-1)}

echo

echo '--- Large Negative Numbers (should stay integer if in range) ---'
echo 'Testing ${-1000000 + 500000}:'
echo ${-1000000 + 500000}

echo 'Testing ${-50000 × 20}:'
echo ${-50000 × 20}

echo

echo '--- Float Fallback Cases (mixed int/float) ---'
echo 'Testing ${-5 + 2.5}:'
echo ${-5 + 2.5}

echo 'Testing ${(-3) × 1.5}:'
echo ${(-3) × 1.5}

echo 'Testing ${-10 ÷ 4.0}:'
echo ${-10 ÷ 4.0}

echo

echo '--- Edge Cases with Very Large Numbers ---'
echo 'Testing overflow behavior...'

# Test near LONG_MAX and LONG_MIN
# LONG_MAX is typically 9223372036854775807 on 64-bit systems
# LONG_MIN is typically -9223372036854775808 on 64-bit systems

echo 'Testing large negative addition:'
echo ${-9223372036854775800 + (-5)}

echo 'Testing large negative multiplication:'
echo ${-1000000000 × 1000000}

echo

echo '--- Comparison Operations with Negatives ---'
echo 'Testing ${-5 > -10} (should be true):'
echo ${-5 > -10}

echo 'Testing ${-3 < 0} (should be true):'  
echo ${-3 < 0}

echo 'Testing ${-7 = -7} (should be true):'
echo ${-7 = -7}

echo

echo '--- Smart Formatting Verification ---'
echo 'These should display as integers:'
echo ${-5 × 2}        # Should show -10
echo ${-15 ÷ 3}       # Should show -5  
echo ${(-4) + (-6)}   # Should show -10

echo
echo 'These should display as floats:'
echo ${-5 ÷ 2}        # Should show -2.5
echo ${-7 × 1.5}      # Should show -10.5

echo

echo '--- Bitwise Operations (should work with negatives) ---'
echo 'Testing ${$&and -8 7}:'
echo ${$&and -8 7}

echo 'Testing ${$&or -1 2}:'
echo ${$&or -1 2}

echo 'Testing ${$&xor -5 3}:'
echo ${$&xor -5 3}

echo 'Testing ${$&not -1}:'
echo ${$&not -1}

echo
echo '=== Negative Number Tests Complete ==='