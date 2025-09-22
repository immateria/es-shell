#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'USAGE'
Usage: $0 [--static] [--output-dir DIR]
Install build dependencies, configure, build, and test es-shell.
Automatically detects Linux (apt-get) or macOS (Homebrew) and uses appropriate package manager.

Options:
  --static          Attempt to link es statically
  --output-dir DIR  Directory to place built binary (default: bin)
  -h, --help        Show this help message and exit
USAGE
}

static=0
out_dir="bin"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --static)
            static=1
            shift
            ;;
        --output-dir)
            out_dir="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac

done

# Detect operating system
detect_os() {
    case "$(uname -s)" in
        Linux*)
            echo "linux"
            ;;
        Darwin*)
            echo "macos" 
            ;;
        *)
            echo "unsupported"
            ;;
    esac
}

# Install dependencies on Linux using apt-get
install_deps_linux() {
    echo "Installing dependencies on Linux using apt-get..."
    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential libtool autoconf automake pkg-config bison flex
}

# Check if Homebrew is installed
check_homebrew() {
    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        
        # Add Homebrew to PATH for current session
        if [[ -f "/opt/homebrew/bin/brew" ]]; then
            # Apple Silicon Mac
            eval "$(/opt/homebrew/bin/brew shellenv)"
        elif [[ -f "/usr/local/bin/brew" ]]; then
            # Intel Mac
            eval "$(/usr/local/bin/brew shellenv)"
        fi
    fi
}

# Check if Xcode command line tools are installed
check_xcode_tools() {
    if ! xcode-select -p >/dev/null 2>&1; then
        echo "Xcode command line tools not found. Installing..."
        xcode-select --install
        
        # Wait for installation to complete
        echo "Please complete the Xcode command line tools installation in the popup window."
        echo "Press any key to continue after installation is complete..."
        read -n 1 -s
    fi
}

# Install dependencies on macOS using Homebrew
install_deps_macos() {
    echo "Installing dependencies on macOS using Homebrew..."
    
    check_xcode_tools
    check_homebrew
    
    # Install required packages
    local packages=("autoconf" "automake" "libtool" "pkg-config" "bison" "flex")
    for package in "${packages[@]}"; do
        if ! brew list "$package" >/dev/null 2>&1; then
            echo "Installing $package..."
            brew install "$package"
        else
            echo "$package already installed"
        fi
    done
    
    # Add bison and other tools to PATH (bison is keg-only)
    if [[ -d "/opt/homebrew/opt/bison/bin" ]]; then
        # Apple Silicon
        export PATH="/opt/homebrew/opt/bison/bin:$PATH"
    elif [[ -d "/usr/local/opt/bison/bin" ]]; then
        # Intel Mac
        export PATH="/usr/local/opt/bison/bin:$PATH"
    fi
    
    # Also ensure libtoolize is in PATH
    if [[ -d "/opt/homebrew/bin" ]] && [[ ! ":$PATH:" == *":/opt/homebrew/bin:"* ]]; then
        export PATH="/opt/homebrew/bin:$PATH"
    elif [[ -d "/usr/local/bin" ]] && [[ ! ":$PATH:" == *":/usr/local/bin:"* ]]; then
        export PATH="/usr/local/bin:$PATH"
    fi
}

# Main dependency installation function
install_dependencies() {
    local os_type
    os_type=$(detect_os)
    
    case "$os_type" in
        linux)
            install_deps_linux
            ;;
        macos)
            install_deps_macos
            ;;
        unsupported)
            echo "Error: Unsupported operating system: $(uname -s)" >&2
            echo "This script supports Linux and macOS only." >&2
            echo "Please install dependencies manually:" >&2
            echo "  - autoconf, automake, libtool, pkg-config, bison, flex" >&2
            echo "  - A C compiler and build tools" >&2
            exit 1
            ;;
    esac
}

# Only run the main script if this file is executed directly, not sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    set -x
    install_dependencies
    
    # Use glibtoolize on macOS, libtoolize on Linux
    if [[ "$(detect_os)" == "macos" ]]; then
        glibtoolize -qi
    else
        libtoolize -qi
    fi
    
    autoreconf -i
    if [[ $static -eq 1 ]]; then
        ./configure LDFLAGS="-static"
    else
        ./configure
    fi
    make clean
    make
    make test
    mkdir -p "$out_dir"
    cp es "$out_dir/es-shell"
fi
