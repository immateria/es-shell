#!/bin/sh
set -u

ES_BINARY=${ES_BINARY:-./es}
LOG_FILE=${LOG_FILE:-test/logs/math-bitwise-primitives.log}

log_dir=$(dirname "$LOG_FILE")
mkdir -p "$log_dir"

if command -v date >/dev/null 2>&1; then
    timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ" 2>/dev/null || date +"%Y-%m-%dT%H:%M:%SZ")
else
    timestamp="unknown"
fi

{
    printf 'Math & Bitwise Primitive Manual Test Log\n'
    printf 'Generated: %s\n\n' "$timestamp"
} > "$LOG_FILE"

run_test() {
    description=$1
    command_text=$2
    expected=$3

    printf 'Test: %s\n' "$description" >> "$LOG_FILE"
    printf 'Command: %s\n' "$command_text" >> "$LOG_FILE"
    printf 'Expected: %s\n' "$expected" >> "$LOG_FILE"

    output=$(/bin/sh -c "$command_text" 2>&1)
    status=$?

    printf 'Actual: %s\n' "$output" >> "$LOG_FILE"
    printf 'Exit status: %d\n' "$status" >> "$LOG_FILE"

    if [ "$status" -eq 0 ] && [ "$output" = "$expected" ]; then
        printf 'Result: PASS\n\n' >> "$LOG_FILE"
    else
        printf 'Result: FAIL\n\n' >> "$LOG_FILE"
    fi
}

run_failure_test() {
    description=$1
    command_text=$2
    expected=$3

    printf 'Test: %s\n' "$description" >> "$LOG_FILE"
    printf 'Command: %s\n' "$command_text" >> "$LOG_FILE"
    printf 'Expected failure output: %s\n' "$expected" >> "$LOG_FILE"

    output=$(/bin/sh -c "$command_text" 2>&1)
    status=$?

    printf 'Actual: %s\n' "$output" >> "$LOG_FILE"
    printf 'Exit status: %d\n' "$status" >> "$LOG_FILE"

    if [ "$status" -ne 0 ] && [ "$output" = "$expected" ]; then
        printf 'Result: PASS\n\n' >> "$LOG_FILE"
    else
        printf 'Result: FAIL\n\n' >> "$LOG_FILE"
    fi
}

run_test 'Prefix addition primitive' "$ES_BINARY -c \"echo <={%addition 1 2}\"" '3'
run_test 'Prefix addition primitive (negative operands)' "$ES_BINARY -c \"echo <={%addition -5 -7}\"" '-12'
run_test 'Prefix subtraction primitive' "$ES_BINARY -c \"echo <={%subtraction 5 2}\"" '3'
run_test 'Prefix multiplication primitive' "$ES_BINARY -c \"echo <={%multiplication 3 4}\"" '12'
run_test 'Prefix multiplication primitive (mixed sign)' "$ES_BINARY -c \"echo <={%multiplication -6 4}\"" '-24'
run_test 'Prefix division primitive' "$ES_BINARY -c \"echo <={%division 8 2}\"" '4'
run_test 'Prefix modulo primitive' "$ES_BINARY -c \"echo <={%modulo 9 4}\"" '1'
run_test 'Prefix power primitive' "$ES_BINARY -c \"echo <={%pow 2 3}\"" '8'
run_test 'Prefix power primitive (negative exponent)' "$ES_BINARY -c \"echo <={%pow 3 -12}\"" '1.88167642315892e-06'
run_test 'Prefix abs primitive' "$ES_BINARY -c \"echo <={%abs -7}\"" '7'
run_test 'Abs word command' "$ES_BINARY -c \"echo <={abs -7}\"" '7'
run_test 'Abs word with grouped infix operand' "$ES_BINARY -c \"echo <={abs (5 minus 9)}\"" '4'
run_test 'Prefix min primitive' "$ES_BINARY -c \"echo <={%min 7 3 5}\"" '3'
run_test 'Prefix max primitive' "$ES_BINARY -c \"echo <={%max 7 3 5}\"" '7'
run_test 'Prefix count primitive' "$ES_BINARY -c \"echo <={%count 1 2 3}\"" '3'
run_test 'Count word command' "$ES_BINARY -c \"echo <={count 1 2 3}\"" '3'
run_test 'Prefix bitwise shift left primitive' "$ES_BINARY -c \"echo <={%bitwiseshiftleft 1 3}\"" '8'
run_test 'Prefix bitwise shift right primitive' "$ES_BINARY -c \"echo <={%bitwiseshiftright 8 1}\"" '4'
run_test 'Prefix bitwise and primitive' "$ES_BINARY -c \"echo <={%bitwiseand 6 3}\"" '2'
run_test 'Prefix bitwise or primitive' "$ES_BINARY -c \"echo <={%bitwiseor 4 1}\"" '5'
run_test 'Prefix bitwise xor primitive' "$ES_BINARY -c \"echo <={%bitwisexor 5 3}\"" '6'
run_test 'Prefix bitwise not primitive' "$ES_BINARY -c \"echo <={%bitwisenot 0}\"" '-1'
run_test 'Prefix division nested with addition' "$ES_BINARY -c \"echo <={%division 10 <={%addition 0 2}}\"" '5'
run_test 'Prefix shift left nested with addition' "$ES_BINARY -c \"echo <={%bitwiseshiftleft 1 <={%addition 1 1}}\"" '4'

run_test 'Addition primitive (var first)' "$ES_BINARY -c 'X = 3
echo <={%addition \$X 4}'" '7'
run_test 'Addition primitive (var second)' "$ES_BINARY -c 'Y = 4
echo <={%addition 3 \$Y}'" '7'
run_test 'Addition primitive (both vars)' "$ES_BINARY -c 'X = 3
Y = 4
echo <={%addition \$X \$Y}'" '7'
run_test 'Subtraction primitive (var first)' "$ES_BINARY -c 'X = 10
echo <={%subtraction \$X 4}'" '6'
run_test 'Subtraction primitive (var second)' "$ES_BINARY -c 'Y = 4
echo <={%subtraction 10 \$Y}'" '6'
run_test 'Subtraction primitive (both vars)' "$ES_BINARY -c 'X = 10
Y = 4
echo <={%subtraction \$X \$Y}'" '6'
run_test 'Multiplication primitive (var first)' "$ES_BINARY -c 'X = 6
echo <={%multiplication \$X 5}'" '30'
run_test 'Multiplication primitive (var second)' "$ES_BINARY -c 'Y = 5
echo <={%multiplication 6 \$Y}'" '30'
run_test 'Multiplication primitive (both vars)' "$ES_BINARY -c 'X = 6
Y = 5
echo <={%multiplication \$X \$Y}'" '30'
run_test 'Division primitive (var first)' "$ES_BINARY -c 'X = 20
echo <={%division \$X 5}'" '4'
run_test 'Division primitive (var second)' "$ES_BINARY -c 'Y = 5
echo <={%division 20 \$Y}'" '4'
run_test 'Division primitive (both vars)' "$ES_BINARY -c 'X = 20
Y = 5
echo <={%division \$X \$Y}'" '4'
run_test 'Modulo primitive (var first)' "$ES_BINARY -c 'X = 17
echo <={%modulo \$X 5}'" '2'
run_test 'Modulo primitive (var second)' "$ES_BINARY -c 'Y = 5
echo <={%modulo 17 \$Y}'" '2'
run_test 'Modulo primitive (both vars)' "$ES_BINARY -c 'X = 17
Y = 5
echo <={%modulo \$X \$Y}'" '2'
run_test 'Power primitive (base var)' "$ES_BINARY -c 'BASE = 2
echo <={%pow \$BASE 5}'" '32'
run_test 'Power primitive (exponent var)' "$ES_BINARY -c 'EXP = 5
echo <={%pow 2 \$EXP}'" '32'
run_test 'Power primitive (both vars)' "$ES_BINARY -c 'BASE = 2
EXP = 5
echo <={%pow \$BASE \$EXP}'" '32'
run_test 'Abs primitive (var input)' "$ES_BINARY -c 'VALUE = -11
echo <={%abs \$VALUE}'" '11'
run_test 'Min primitive (var inputs)' "$ES_BINARY -c 'A = 9
B = 2
echo <={%min \$A \$B}'" '2'
run_test 'Max primitive (var inputs)' "$ES_BINARY -c 'A = 9
B = 2
echo <={%max \$A \$B}'" '9'
run_test 'Count primitive (list var)' "$ES_BINARY -c 'LIST = a b c d
echo <={%count \$LIST}'" '4'
run_test 'Bitwise and primitive (var first)' "$ES_BINARY -c 'X = 6
echo <={%bitwiseand \$X 3}'" '2'
run_test 'Bitwise and primitive (var second)' "$ES_BINARY -c 'Y = 3
echo <={%bitwiseand 6 \$Y}'" '2'
run_test 'Bitwise and primitive (both vars)' "$ES_BINARY -c 'X = 6
Y = 3
echo <={%bitwiseand \$X \$Y}'" '2'
run_test 'Bitwise or primitive (var first)' "$ES_BINARY -c 'X = 4
echo <={%bitwiseor \$X 1}'" '5'
run_test 'Bitwise or primitive (var second)' "$ES_BINARY -c 'Y = 1
echo <={%bitwiseor 4 \$Y}'" '5'
run_test 'Bitwise or primitive (both vars)' "$ES_BINARY -c 'X = 4
Y = 1
echo <={%bitwiseor \$X \$Y}'" '5'
run_test 'Bitwise xor primitive (var first)' "$ES_BINARY -c 'X = 5
echo <={%bitwisexor \$X 3}'" '6'
run_test 'Bitwise xor primitive (var second)' "$ES_BINARY -c 'Y = 3
echo <={%bitwisexor 5 \$Y}'" '6'
run_test 'Bitwise xor primitive (both vars)' "$ES_BINARY -c 'X = 5
Y = 3
echo <={%bitwisexor \$X \$Y}'" '6'
run_test 'Bitwise shift left primitive (var first)' "$ES_BINARY -c 'BASE = 2
echo <={%bitwiseshiftleft \$BASE 3}'" '16'
run_test 'Bitwise shift left primitive (var second)' "$ES_BINARY -c 'SHIFT = 3
echo <={%bitwiseshiftleft 2 \$SHIFT}'" '16'
run_test 'Bitwise shift left primitive (both vars)' "$ES_BINARY -c 'BASE = 2
SHIFT = 3
echo <={%bitwiseshiftleft \$BASE \$SHIFT}'" '16'
run_test 'Bitwise shift right primitive (var first)' "$ES_BINARY -c 'BASE = 32
echo <={%bitwiseshiftright \$BASE 2}'" '8'
run_test 'Bitwise shift right primitive (var second)' "$ES_BINARY -c 'SHIFT = 2
echo <={%bitwiseshiftright 32 \$SHIFT}'" '8'
run_test 'Bitwise shift right primitive (both vars)' "$ES_BINARY -c 'BASE = 32
SHIFT = 2
echo <={%bitwiseshiftright \$BASE \$SHIFT}'" '8'

run_test 'Infix addition token' "$ES_BINARY -c \"echo <={1 plus 2}\"" '3'
run_test 'Infix subtraction token (minus)' "$ES_BINARY -c \"echo <={5 minus 2}\"" '3'
run_test 'Infix subtraction token (subtract)' "$ES_BINARY -c \"echo <={9 subtract 4}\"" '5'
run_test 'Infix multiplication token' "$ES_BINARY -c \"echo <={3 multiply 4}\"" '12'
run_test 'Infix division token' "$ES_BINARY -c \"echo <={8 divide 2}\"" '4'
run_test 'Infix multiplication alternate word' "$ES_BINARY -c \"echo <={3 multiplied-by 4}\"" '12'
run_test 'Infix division alternate word' "$ES_BINARY -c \"echo <={8 divided-by 2}\"" '4'
run_test 'Infix modulo word' "$ES_BINARY -c \"echo <={9 mod 4}\"" '1'
run_test 'Infix power word' "$ES_BINARY -c \"echo <={2 power 5}\"" '32'
run_test 'Infix power alternate word' "$ES_BINARY -c \"echo <={2 raised-to 3}\"" '8'
run_test 'Infix addition with negative operands' "$ES_BINARY -c \"echo <={-4 plus -6}\"" '-10'
run_test 'Infix subtraction with negative right operand' "$ES_BINARY -c \"echo <={5 minus -3}\"" '8'
run_test 'Infix power negative exponent' "$ES_BINARY -c \"echo <={3 power -12}\"" '1.88167642315892e-06'
run_test 'Infix min word' "$ES_BINARY -c \"echo <={7 minimum 3}\"" '3'
run_test 'Infix max word' "$ES_BINARY -c \"echo <={7 maximum 3}\"" '7'

run_test 'Addition infix (var first)' "$ES_BINARY -c 'X = 3
echo <={\$X plus 4}'" '7'
run_test 'Addition infix (var second)' "$ES_BINARY -c 'Y = 4
echo <={3 plus \$Y}'" '7'
run_test 'Addition infix (both vars)' "$ES_BINARY -c 'X = 3
Y = 4
echo <={\$X plus \$Y}'" '7'
run_test 'Subtraction infix (var first)' "$ES_BINARY -c 'X = 10
echo <={\$X minus 4}'" '6'
run_test 'Subtraction infix (var second)' "$ES_BINARY -c 'Y = 4
echo <={10 minus \$Y}'" '6'
run_test 'Subtraction infix (both vars)' "$ES_BINARY -c 'X = 10
Y = 4
echo <={\$X minus \$Y}'" '6'
run_test 'Multiplication infix (var first)' "$ES_BINARY -c 'X = 6
echo <={\$X multiply 5}'" '30'
run_test 'Multiplication infix (var second)' "$ES_BINARY -c 'Y = 5
echo <={6 multiply \$Y}'" '30'
run_test 'Multiplication infix (both vars)' "$ES_BINARY -c 'X = 6
Y = 5
echo <={\$X multiply \$Y}'" '30'
run_test 'Division infix (var first)' "$ES_BINARY -c 'X = 20
echo <={\$X divide 5}'" '4'
run_test 'Division infix (var second)' "$ES_BINARY -c 'Y = 5
echo <={20 divide \$Y}'" '4'
run_test 'Division infix (both vars)' "$ES_BINARY -c 'X = 20
Y = 5
echo <={\$X divide \$Y}'" '4'
run_test 'Modulo infix (var first)' "$ES_BINARY -c 'X = 17
echo <={\$X mod 5}'" '2'
run_test 'Modulo infix (var second)' "$ES_BINARY -c 'Y = 5
echo <={17 mod \$Y}'" '2'
run_test 'Modulo infix (both vars)' "$ES_BINARY -c 'X = 17
Y = 5
echo <={\$X mod \$Y}'" '2'
run_test 'Power infix (base var)' "$ES_BINARY -c 'BASE = 2
echo <={\$BASE power 5}'" '32'
run_test 'Power infix (exponent var)' "$ES_BINARY -c 'EXP = 5
echo <={2 power \$EXP}'" '32'
run_test 'Power infix (both vars)' "$ES_BINARY -c 'BASE = 2
EXP = 5
echo <={\$BASE power \$EXP}'" '32'
run_test 'Power infix alternate word with vars' "$ES_BINARY -c 'BASE = 3
EXP = 3
echo <={\$BASE raised-to \$EXP}'" '27'
run_test 'Min infix (var first)' "$ES_BINARY -c 'A = 9
echo <={\$A minimum 4}'" '4'
run_test 'Min infix (var second)' "$ES_BINARY -c 'B = 4
echo <={9 minimum \$B}'" '4'
run_test 'Min infix (both vars)' "$ES_BINARY -c 'A = 9
B = 4
echo <={\$A minimum \$B}'" '4'
run_test 'Max infix (var first)' "$ES_BINARY -c 'A = 9
echo <={\$A maximum 4}'" '9'
run_test 'Max infix (var second)' "$ES_BINARY -c 'B = 4
echo <={9 maximum \$B}'" '9'
run_test 'Max infix (both vars)' "$ES_BINARY -c 'A = 9
B = 4
echo <={\$A maximum \$B}'" '9'
run_test 'Bitwise infix and (~AND) var first' "$ES_BINARY -c 'X = 6
echo <={\$X ~AND 3}'" '2'
run_test 'Bitwise infix and (~AND) var second' "$ES_BINARY -c 'Y = 3
echo <={6 ~AND \$Y}'" '2'
run_test 'Bitwise infix and (~AND) both vars' "$ES_BINARY -c 'X = 6
Y = 3
echo <={\$X ~AND \$Y}'" '2'
run_test 'Bitwise infix or (~OR) var first' "$ES_BINARY -c 'X = 4
echo <={\$X ~OR 1}'" '5'
run_test 'Bitwise infix or (~OR) var second' "$ES_BINARY -c 'Y = 1
echo <={4 ~OR \$Y}'" '5'
run_test 'Bitwise infix or (~OR) both vars' "$ES_BINARY -c 'X = 4
Y = 1
echo <={\$X ~OR \$Y}'" '5'
run_test 'Bitwise infix xor (~XOR) var first' "$ES_BINARY -c 'X = 5
echo <={\$X ~XOR 3}'" '6'
run_test 'Bitwise infix xor (~XOR) var second' "$ES_BINARY -c 'Y = 3
echo <={5 ~XOR \$Y}'" '6'
run_test 'Bitwise infix xor (~XOR) both vars' "$ES_BINARY -c 'X = 5
Y = 3
echo <={\$X ~XOR \$Y}'" '6'
run_test 'Bitwise shift left (~SHL) var first' "$ES_BINARY -c 'BASE = 2
echo <={\$BASE ~SHL 3}'" '16'
run_test 'Bitwise shift left (~SHL) var second' "$ES_BINARY -c 'SHIFT = 3
echo <={2 ~SHL \$SHIFT}'" '16'
run_test 'Bitwise shift left (~SHL) both vars' "$ES_BINARY -c 'BASE = 2
SHIFT = 3
echo <={\$BASE ~SHL \$SHIFT}'" '16'
run_test 'Bitwise shift right (~SHR) var first' "$ES_BINARY -c 'BASE = 32
echo <={\$BASE ~SHR 2}'" '8'
run_test 'Bitwise shift right (~SHR) var second' "$ES_BINARY -c 'SHIFT = 2
echo <={32 ~SHR \$SHIFT}'" '8'
run_test 'Bitwise shift right (~SHR) both vars' "$ES_BINARY -c 'BASE = 32
SHIFT = 2
echo <={\$BASE ~SHR \$SHIFT}'" '8'

run_test 'Infix division grouped with addition' "$ES_BINARY -c \"echo <={10 divide (0 plus 2)}\"" '5'
run_test 'Infix shift-left grouped with addition' "$ES_BINARY -c \"echo <={4 shift-left (1 plus 1)}\"" '16'
run_failure_test 'Ungrouped divide by zero propagates' "$ES_BINARY -c \"echo <={10 divide 0 plus 2}\"" 'division by zero'

run_test 'Bitwise infix and (~AND)' "$ES_BINARY -c \"echo <={6 ~AND 3}\"" '2'
run_test 'Bitwise infix or (~OR)' "$ES_BINARY -c \"echo <={4 ~OR 1}\"" '5'
run_test 'Bitwise infix xor (~XOR)' "$ES_BINARY -c \"echo <={5 ~XOR 3}\"" '6'
run_test 'Bitwise prefix not (~NOT)' "$ES_BINARY -c \"echo <={~NOT 0}\"" '-1'
run_test 'Bitwise shift left (~SHL)' "$ES_BINARY -c \"echo <={8 ~SHL 2}\"" '32'
run_test 'Bitwise shift right (~SHR)' "$ES_BINARY -c \"echo <={12 ~SHR 2}\"" '3'
run_test 'Bitwise shift with variables (~SHL/~SHR)' \
    "$ES_BINARY test/scripts/bitwise-infix-vars.es" '32 4'
