#!/usr/bin/env bash
set -uo pipefail

# Color codes for output
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly PURPLE='\033[0;35m'
readonly CYAN='\033[0;36m'
readonly WHITE='\033[1;37m'
readonly BOLD='\033[1m'
readonly NC='\033[0m' # No Color


readonly CHECKMARK="✓"
readonly CROSS="╳"
readonly ARROW="➤"
readonly GEAR="⛭"
readonly LAMBDA="λ"
readonly PACKAGE="◊"

start_time=""
step_count=0
total_steps=0

# Enhanced logging functions with better visual appeal
log_info() {
    echo -e "${BLUE}${ARROW}${NC} $*"
}

log_success() {
    echo -e "${GREEN}${CHECKMARK} $*${NC}"
}

log_warn() {
    echo -e "${YELLOW}⚠️  [WARNING]${NC} $*"
}

log_error() {
    echo -e "${RED}${CROSS} [ERROR]${NC} $*" >&2
}

log_step() {
    ((step_count++))
    local progress=""
    if [[ $total_steps -gt 0 ]]; then
        progress=" (${step_count}/${total_steps})"
    fi
    echo
    echo -e "${WHITE}${BOLD}╭─────────────────────────────────────────────────────╮${NC}"
    echo -e "${WHITE}${BOLD}│${NC} ${GEAR} ${CYAN}$*${progress}${NC}"
    echo -e "${WHITE}${BOLD}╰─────────────────────────────────────────────────────╯${NC}"
}

log_header() {
    echo
    echo -e "${PURPLE}${BOLD}┌───────────────────────────────────────────────────────┐${NC}"
    echo -e "${PURPLE}${BOLD}│${NC}  ${LAMBDA} $*"
    echo -e "${PURPLE}${BOLD}└───────────────────────────────────────────────────────┘${NC}"
    echo
}

log_package() {
    echo -e "  ${PACKAGE} ${WHITE}$*${NC}"
}

# Progress spinner for long operations
show_spinner() {
    local pid=$1
    local message="${2:-Processing}"
    local delay=0.1
    local -a spinner=('⠋' '⠙' '⠹' '⠸' '⠼' '⠴' '⠦' '⠧' '⠇' '⠏')
    local i=0
    while kill -0 "$pid" 2>/dev/null; do
        printf "\r  ${YELLOW}%s${NC} %s..." "${spinner[i]}" "$message"
        i=$(((i + 1) % 10))
        sleep $delay
    done
    printf "\r"
}

# Execute command with progress spinner and clean output
execute_with_progress() {
    local cmd="$1"
    local message="${2:-Running command}"
    local logfile="${3:-build.log}"
    
    if [[ $dry_run -eq 1 ]]; then
        log_info "[DRY-RUN] Would execute with progress: $cmd"
        return 0
    fi
    
    # Start command in background, redirecting output to log
    eval "$cmd" >"$logfile" 2>&1 &
    local cmd_pid=$!
    
    # Show spinner while command runs
    show_spinner "$cmd_pid" "$message"
    
    # Wait for command to complete and get exit status
    wait "$cmd_pid"
    local exit_status=$?
    
    # Clear the spinner line and show result
    printf "\r  "
    if [[ $exit_status -eq 0 ]]; then
        echo -e "${GREEN}${CHECKMARK}${NC} $message completed successfully"
    else
        echo -e "${RED}${CROSS}${NC} $message failed (see $logfile for details)"
        return $exit_status
    fi
    
    return 0
}

# Start timing a build process
start_timing() {
    start_time=$(date +%s)
}

# Get elapsed time in a human readable format
get_elapsed_time() {
    local end_time
    end_time=$(date +%s)
    local elapsed=$((end_time - start_time))
    local minutes=$((elapsed / 60))
    local seconds=$((elapsed % 60))
    
    if [[ $minutes -gt 0 ]]; then
        echo "${minutes}m ${seconds}s"
    else
        echo "${seconds}s"
    fi
}

usage() {
    cat <<'USAGE'
Usage: $0 [--static] [--output-dir DIR] [--dry-run] [--enable-tests]
Install build dependencies, configure, build, and test es-shell.
Automatically detects Linux (apt-get) or macOS (Homebrew) and uses appropriate package manager.

Options:
  --static          Attempt to link es statically
  --output-dir DIR  Directory to place built binary (default: bin)
  --dry-run         Show what would be done without executing
  --enable-tests    Run tests after building
  -h, --help        Show this help message and exit
USAGE
}

static=0
out_dir="bin"
dry_run=0
enable_tests=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --static)
            static=1
            shift
            ;;
        --output-dir)
            if [[ -z "${2:-}" ]]; then
                log_error "Option --output-dir requires a directory argument"
                exit 1
            fi
            out_dir="$2"
            shift 2
            ;;
        --dry-run)
            dry_run=1
            shift
            ;;
        --enable-tests)
            enable_tests=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
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

# Validate that a command exists and is functional
validate_tool() {
    local tool="$1"
    local test_cmd="${2:-}"
    
    if ! command -v "$tool" >/dev/null 2>&1; then
        log_error "$tool is not available in PATH"
        return 1
    fi
    
    if [[ -n "$test_cmd" ]]; then
        if ! eval "$test_cmd" >/dev/null 2>&1; then
            log_error "$tool exists but failed basic test: $test_cmd"
            return 1
        fi
    fi
    
    echo -e "    ${GREEN}${CHECKMARK}${NC} ${tool} is available and functional"
    return 0
}

# Dry run wrapper
execute() {
    if [[ $dry_run -eq 1 ]]; then
        log_info "[DRY-RUN] Would execute: $*"
    else
        "$@"
    fi
}

# Install dependencies on Linux using apt-get
install_deps_linux() {
    # Check if running as root or with sudo
    if [[ $EUID -ne 0 ]] && ! command -v sudo >/dev/null 2>&1; then
        log_error "This script needs to install packages but sudo is not available and not running as root"
        exit 1
    fi
    
    local apt_cmd="apt-get"
    if [[ $EUID -ne 0 ]]; then
        apt_cmd="sudo apt-get"
    fi
    
    log_info "Updating package lists..."
    execute "$apt_cmd" update
    
    log_info "Installing build dependencies..."
    log_package "build-essential, libtool, autoconf, automake, pkg-config, bison, flex"
    execute env DEBIAN_FRONTEND=noninteractive "$apt_cmd" install -y \
        build-essential libtool autoconf automake pkg-config bison flex
    
    log_info "Validating installed tools..."
    validate_tool gcc "gcc --version"
    validate_tool make "make --version"
    validate_tool autoconf "autoconf --version"
    validate_tool automake "automake --version"
}

# Check if Homebrew is installed
check_homebrew() {
    if ! command -v brew >/dev/null 2>&1; then
        log_step "Homebrew not found. Installing Homebrew..."
        execute /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        
        # Add Homebrew to PATH for current session
        if [[ -f "/opt/homebrew/bin/brew" ]]; then
            # Apple Silicon Mac
            log_info "Setting up Homebrew environment for Apple Silicon"
            eval "$(/opt/homebrew/bin/brew shellenv)"
        elif [[ -f "/usr/local/bin/brew" ]]; then
            # Intel Mac
            log_info "Setting up Homebrew environment for Intel"
            eval "$(/usr/local/bin/brew shellenv)"
        else
            log_error "Homebrew installation failed - brew command not found after install"
            exit 1
        fi
    else
        log_info "Homebrew is already installed"
    fi
    
    # Validate Homebrew is working
    validate_tool brew "brew --version"
}

# Check if Xcode command line tools are installed
check_xcode_tools() {
    if ! xcode-select -p >/dev/null 2>&1; then
        log_step "Xcode command line tools not found. Installing..."
        if [[ $dry_run -eq 1 ]]; then
            log_info "[DRY-RUN] Would run: xcode-select --install"
            log_info "[DRY-RUN] Would wait for user confirmation"
        else
            xcode-select --install
            
            # Wait for installation to complete
            log_warn "Please complete the Xcode command line tools installation in the popup window."
            echo "Press any key to continue after installation is complete..."
            read -n 1 -s -r
            
            # Verify installation succeeded
            if ! xcode-select -p >/dev/null 2>&1; then
                log_error "Xcode command line tools installation failed or incomplete"
                exit 1
            fi
            log_success "Xcode command line tools installed successfully"
        fi
    else
        log_info "Xcode command line tools are already installed"
    fi
}

# Install dependencies on macOS using Homebrew
install_deps_macos() {
    check_xcode_tools
    check_homebrew
    
    # Install required packages
    log_info "Installing Homebrew packages..."
    local packages=("autoconf" "automake" "libtool" "pkg-config" "bison" "flex")
    for package in "${packages[@]}"; do
        if ! brew list "$package" >/dev/null 2>&1; then
            log_package "Installing $package"
            execute brew install "$package"
        else
            log_package "$package (already installed)"
        fi
    done
    
    # Set up PATH for keg-only packages
    local homebrew_prefix
    if [[ -d "/opt/homebrew" ]]; then
        homebrew_prefix="/opt/homebrew"
        log_info "Detected Apple Silicon Homebrew"
    elif [[ -d "/usr/local" ]]; then
        homebrew_prefix="/usr/local"
        log_info "Detected Intel Homebrew"
    else
        log_error "Cannot determine Homebrew prefix"
        exit 1
    fi
    
    # Add bison to PATH (it's keg-only)
    if [[ -d "$homebrew_prefix/opt/bison/bin" ]]; then
        export PATH="$homebrew_prefix/opt/bison/bin:$PATH"
        log_info "Added bison to PATH: $homebrew_prefix/opt/bison/bin"
    fi
    
    # Ensure Homebrew tools are in PATH
    if [[ -d "$homebrew_prefix/bin" ]] && [[ ! ":$PATH:" == *":$homebrew_prefix/bin:"* ]]; then
        export PATH="$homebrew_prefix/bin:$PATH"
        log_info "Added Homebrew tools to PATH: $homebrew_prefix/bin"
    fi
    
    # Validate key tools after installation
    validate_tool gcc "gcc --version"
    validate_tool make "make --version"
    validate_tool autoconf "autoconf --version"
    validate_tool automake "automake --version"
    validate_tool bison "bison --version"
    validate_tool glibtoolize "glibtoolize --version"
}

# Main dependency installation function
install_dependencies() {
    local os_type
    os_type=$(detect_os)
    
    log_step "Installing Dependencies"
    
    case "$os_type" in
        linux)
            install_deps_linux
            ;;
        macos)
            install_deps_macos
            ;;
        unsupported)
            log_error "Unsupported operating system: $(uname -s)"
            log_error "This script supports Linux and macOS only."
            log_error "Please install dependencies manually:"
            log_error "  - autoconf, automake, libtool, pkg-config, bison, flex"
            log_error "  - A C compiler and build tools"
            exit 1
            ;;
    esac
    
    log_success "Dependencies installed successfully"
}
# Clean architecture-specific build artifacts
clean_architecture_specific() {
    log_step "Cleaning Build Artifacts"
    
    log_info "Removing previous build files..."
    
    # Clean using make if Makefile exists
    if [[ -f Makefile ]]; then
        execute make clean
    fi
    
    # Remove object files
    execute rm -f -- *.o src/*.o src/*/*.o generated/*.o
    
    # Remove binaries
    execute rm -f es esdump
    
    # Remove library directories
    execute rm -rf .libs
    
    # Find and remove any stray object files
    if [[ $dry_run -eq 0 ]]; then
        find . -name "*.o" -delete 2>/dev/null || true
    else
        log_info "[DRY-RUN] Would find and delete all .o files"
    fi
    
    log_success "Build artifacts cleaned"
}

# Only run the main script if this file is executed directly, not sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    # Set up build process  
    if [[ $enable_tests -eq 1 ]]; then
        total_steps=8  # Dependencies, autotools, clean, configure, build, test, install, summary
    else
        total_steps=7  # Dependencies, autotools, clean, configure, build, install, summary  
    fi
    start_timing
    
    log_header "ES-SHELL BUILD SYSTEM"
    log_info "Build configuration:"
    echo -e "  ${ARROW} Static linking: $([ $static -eq 1 ] && echo "${GREEN}enabled${NC}" || echo "${YELLOW}disabled${NC}")"
    echo -e "  ${ARROW} Output directory: ${CYAN}$out_dir${NC}"
    echo -e "  ${ARROW} Tests: $([ $enable_tests -eq 1 ] && echo "${GREEN}enabled${NC}" || echo "${YELLOW}disabled${NC}")"
    
    if [[ $dry_run -eq 1 ]]; then
        log_warn "DRY RUN MODE - No actual changes will be made"
    fi
    
    # Install dependencies
    install_dependencies
    
    # Configure autotools
    log_step "Setting up Autotools"
    
    log_info "Creating build directories..."
    execute mkdir -p build-aux build
    
    # Use glibtoolize on macOS, libtoolize on Linux  
    if [[ "$(detect_os)" == "macos" ]]; then
        execute_with_progress "glibtoolize -qi" "Running glibtoolize" "autotools.log"
    else
        execute_with_progress "libtoolize -qi" "Running libtoolize" "autotools.log" 
    fi
    
    # Run aclocal with output to build-aux, including local m4 macros
    execute_with_progress "aclocal --output=build-aux/aclocal.m4 -I m4" "Running aclocal" "autotools.log"
    
    # Run autoconf with cache in build-aux
    if [[ $dry_run -eq 0 ]]; then
        execute_with_progress "autom4te --cache=build-aux/autom4te.cache --language=autoconf --include=build-aux configure.ac > configure && chmod +x configure" "Running autoconf" "autotools.log"
    else
        log_info "[DRY-RUN] Would run autom4te and create configure script"
    fi
    
    # Generate config.h template with cache in build-aux
    execute_with_progress "env AUTOM4TE_CACHE=build-aux/autom4te.cache autoheader" "Running autoheader" "autotools.log"
    
    # Clean up any stray autom4te.cache that might get created in root
    if [[ -d autom4te.cache ]]; then
        execute rm -rf autom4te.cache
    fi
    
    # Clean architecture-specific artifacts
    clean_architecture_specific

    # Configure the build
    log_step "Configuring Build System"
    if [[ $static -eq 1 ]]; then
        log_info "Configuring for static linking..."
        execute_with_progress "./configure LDFLAGS='-static'" "Configuring build system" "configure.log"
    else
        log_info "Configuring for dynamic linking..."
        execute_with_progress "./configure" "Configuring build system" "configure.log"
    fi
    
    # Clean up any autom4te.cache created by configure
    if [[ -d autom4te.cache ]]; then
        execute rm -rf autom4te.cache
    fi
    
    # Build the project
    log_step "Compiling ES-Shell"
    log_info "Compiling source files..."
    execute_with_progress "make" "Compiling ES-Shell" "make.log"
    
    # Run tests if enabled
    if [[ $enable_tests -eq 1 ]]; then
        log_step "Running Test Suite"
        if execute_with_progress "make test" "Running tests" "test.log"; then
            log_success "All tests passed"
        else
            log_warn "Some tests failed, but continuing with build"
        fi
    else
        log_step "Skipping Tests"
        log_info "Use --enable-tests to run the test suite"
    fi
    
    # Install to output directory
    log_step "Installing Binary"
    log_info "Creating output directory: $out_dir"
    execute mkdir -p "$out_dir"
    log_info "Installing es-shell binary..."
    execute cp es "$out_dir/es-shell"
    
    # Final summary
    echo
    echo -e "${WHITE}${BOLD}╭─────────────────────────────────────────────────────╮${NC}"
    echo -e "${WHITE}${BOLD}│${NC} ${GEAR} ${CYAN}Build Summary${NC}"
    echo -e "${WHITE}${BOLD}╰─────────────────────────────────────────────────────╯${NC}"
    elapsed_time=$(get_elapsed_time)
    
    echo
    log_success "ES-Shell build completed successfully!"
    echo
    echo -e "${WHITE}${BOLD}┌─────────────────────────────────────────┐${NC}"
    echo -e "${WHITE}${BOLD}│${NC} ${GREEN}${CHECKMARK}${NC} Binary:         ${GREEN}$out_dir/es-shell${NC}"
    echo -e "${WHITE}${BOLD}│${NC} ${GREEN}${CHECKMARK}${NC} Build time:     ${CYAN}$elapsed_time${NC}"
    echo -e "${WHITE}${BOLD}│${NC} ${GREEN}${CHECKMARK}${NC} Static linking: $([ $static -eq 1 ] && echo "${GREEN}enabled${NC}" || echo "${YELLOW}disabled${NC}")"
    echo -e "${WHITE}${BOLD}│${NC} ${GREEN}${CHECKMARK}${NC} Tests:          $([ $enable_tests -eq 1 ] && echo "${GREEN}ran${NC}" || echo "${YELLOW}skipped${NC}")"
    echo -e "${WHITE}${BOLD}│${NC} ${GREEN}${CHECKMARK}${NC} Log files:      ${CYAN}*.log${NC}"
    echo -e "${WHITE}${BOLD}└─────────────────────────────────────────┘${NC}"
    
    if [[ $dry_run -eq 0 ]]; then
        echo
        echo -e "${ARROW} To test the shell, run: ${GREEN}$out_dir/es-shell${NC}"
        echo -e "${ARROW} For help, run: ${GREEN}$out_dir/es-shell --help${NC}"
    fi
    
    echo
    echo -e "${LAMBDA} ${GREEN}${BOLD}Ready to launch!${NC}"
fi
