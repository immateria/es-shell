#!/bin/bash
# debug-build.sh - Easy ES shell debug build and test script

set -e

echo "ES Shell Debug Build Script"
echo "==========================="

# Clean any existing builds
echo "Cleaning previous builds..."
make clean > /dev/null 2>&1 || true

# Build debug version
echo "Building debug version with ES_DEBUG enabled..."
if make debug; then
    echo "✅ Debug build successful! Binary: es-debug"
else
    echo "❌ Debug build failed!"
    exit 1
fi

echo ""
echo "Debug Environment Variables:"
echo "  ES_DEBUG=all       - Enable all debug categories"
echo "  ES_DEBUG=assign    - Enable assignment operator debugging"  
echo "  ES_DEBUG=token     - Enable tokenizer debugging"
echo "  ES_DEBUG=prim      - Enable primitive debugging"
echo "  ES_DEBUG=parse     - Enable parser debugging"
echo "  ES_DEBUG=eval      - Enable evaluation debugging"
echo ""
echo "Example usage:"
echo "  ES_DEBUG=assign ./es-debug -c 'x=5; \${%plus-assign x 3}; echo \$x'"
echo "  ES_DEBUG=all ./es-debug -c 'echo hello'"
echo ""

# Test if user wants to run a quick test
if [ "$1" = "test" ]; then
    echo "Running quick assignment operator test with debug tracing..."
    echo "Command: ES_DEBUG=assign ./es-debug -c 'x=5; \${%plus-assign x 3}; echo \$x'"
    echo ""
    ES_DEBUG=assign ./es-debug -c 'x=5; ${%plus-assign x 3}; echo $x'
fi