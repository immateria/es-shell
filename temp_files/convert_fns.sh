#!/bin/bash

# Convert ES function definitions from 'fn name' to 'fn-name = @' syntax
# This script helps convert the initial.es file

# Usage: ./convert_fns.sh < initial.es > initial_converted.es

sed 's/^fn \([^{]*\){\(.*\)$/fn-\1= @ {\2/' |
sed 's/^fn \([^ ]*\) \([^{]*\){\(.*\)$/fn-\1 = @ \2{\3/'