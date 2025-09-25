# macOS Compatibility Information

## Current Build Configuration

The ES shell build system now supports flexible architecture targeting for maximum compatibility across Mac hardware generations.

### Supported Build Targets

#### Intel-Only Build (`--intel-only`)
- **Architecture**: x86_64 only  
- **Minimum macOS Version**: 10.13 (High Sierra) - September 2017
- **Use Case**: Maximum backward compatibility on Intel Macs
- **Works On**: Intel Macs (2017+), Apple Silicon via Rosetta 2

#### Apple Silicon-Only Build (`--arm-only`)  
- **Architecture**: arm64 only
- **Minimum macOS Version**: 11.0 (Big Sur) - November 2020
- **Use Case**: Native performance on Apple Silicon
- **Works On**: Apple Silicon Macs only

#### Universal Binary Build (`--universal`)
- **Architecture**: x86_64 + arm64 (fat binary)
- **Minimum macOS Version**: 10.13 for Intel slice, 11.0 for ARM slice  
- **Use Case**: Single binary for all modern Macs
- **Works On**: Both Intel and Apple Silicon Macs natively

### Build Details

The build system automatically detects macOS and applies the following settings:

- `MACOSX_DEPLOYMENT_TARGET=10.13`
- Compiler flags: `-Os -fno-common -mmacosx-version-min=10.13`  
- Linker flags: `-Wl,-dead_strip` (removes unused code)

### Dependencies

The ES shell only depends on system libraries available since macOS 10.13:

- `libedit.3.dylib` - System line editing library
- `libSystem.B.dylib` - Core system library

### Version History Support

| macOS Version | Code Name | Supported |
|---------------|-----------|-----------|
| 10.13+        | High Sierra and later | ✅ Yes |
| 10.12         | Sierra | ❌ Not tested |
| 10.11         | El Capitan | ❌ No |
| 10.10 and older | Yosemite and older | ❌ No |

### Why Not Static Linking?

macOS does not support traditional static linking like Linux systems because:

1. Apple does not provide static versions of system libraries (like `crt0.o`)
2. The macOS system integrity requires dynamic linking to system frameworks
3. Static linking would break compatibility with system security features

### Optimized Dynamic Linking

Instead of static linking, the ES shell uses optimized dynamic linking:

- **Dead code elimination** (`-Wl,-dead_strip`) removes unused functions
- **Size optimization** (`-Os`) minimizes binary size  
- **Compatibility flags** (`-fno-common`) ensures broad compatibility

### Building for Different Targets

The build system automatically selects the best target based on your host system, but you can override this:

```bash
# Intel-only build (macOS 10.13+)  
./build.sh --static --intel-only

# Apple Silicon-only build (macOS 11.0+)
./build.sh --static --arm-only

# Universal binary (both architectures)
./build.sh --static --universal

# Auto-detection (default behavior)
./build.sh --static
# Intel Mac → builds --intel-only
# Apple Silicon → builds --universal  
```

### Host System Auto-Detection

- **Intel Mac**: Defaults to `--intel-only` for fastest builds and maximum compatibility
- **Apple Silicon Mac**: Defaults to `--universal` for maximum utility

### Cross-Platform Considerations

- **Intel Mac building Universal**: May require additional setup for ARM64 toolchain
- **Apple Silicon building Intel**: Uses Rosetta 2 for Intel portions automatically
- **Dependencies**: ICU and other x86_64-only libraries are automatically excluded

### Verifying Build Architecture

To check which architectures are included in your binary:

```bash
# Check architecture slices
lipo -info bin/es-shell
# Expected: "Architectures in the fat file: bin/es-shell are: x86_64 arm64"

# Check Intel deployment target (should be 10.13)
lipo -thin x86_64 bin/es-shell -output /tmp/es-x86 && otool -l /tmp/es-x86 | grep -A 3 LC_VERSION_MIN

# Check ARM deployment target (should be 11.0)  
lipo -thin arm64 bin/es-shell -output /tmp/es-arm && otool -l /tmp/es-arm | grep -A 3 LC_BUILD_VERSION
```

### Checking Binary Compatibility

To check the minimum macOS version of a built binary:

```bash
# Method 1: Using vtool
vtool -show-build bin/es-shell

# Method 2: Using otool  
otool -l bin/es-shell | grep -A 5 "LC_VERSION_MIN"
```

## Current Status

✅ **Successfully targeting macOS 10.13+** - This provides compatibility with systems from 2017 onwards while maintaining all functionality.

The binary is approximately **~130KB** and has minimal external dependencies, making it very portable across macOS systems.