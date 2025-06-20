#!/bin/bash

# FG-OS Build Script
# Developed by: Faiz Nasir - FGCompany Official
# Description: Main build script for FG-OS project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project Information
PROJECT_NAME="FG-OS"
PROJECT_VERSION="1.0.0"
BUILD_TYPE="Debug"
TARGET_ARCH="x86_64"

# Build configuration
BUILD_DIR="build"
ISO_DIR="iso"
KERNEL_BIN="kernel.bin"
ISO_FILE="fg-os.iso"

# Function to print status messages
print_status() {
    echo -e "${BLUE}[BUILD]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -r, --release       Build in release mode"
    echo "  -d, --debug         Build in debug mode (default)"
    echo "  -t, --target ARCH   Target architecture (default: x86_64)"
    echo "  -j, --jobs N        Number of parallel build jobs"
    echo "  --iso               Build ISO image"
    echo "  --run               Build and run in QEMU"
    echo "  --test              Run test suite"
    echo ""
    echo "Examples:"
    echo "  $0 --clean --release --iso    # Clean build in release mode with ISO"
    echo "  $0 --debug --run              # Debug build and run in QEMU"
    echo "  $0 --test                     # Run test suite"
}

# Function to clean build directory
clean_build() {
    print_status "Cleaning build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    if [ -d "$ISO_DIR" ]; then
        rm -rf "$ISO_DIR"
    fi
    print_success "Build directory cleaned!"
}

# Function to configure build
configure_build() {
    print_status "Configuring build..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    
    # Configure with CMake
    cmake -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DTARGET_ARCH="$TARGET_ARCH" \
        -DCURRENT_PHASE=1 \
        .
    
    print_success "Build configured successfully!"
}

# Function to build project
build_project() {
    print_status "Building $PROJECT_NAME..."
    
    # Build with CMake
    if [ -n "$JOBS" ]; then
        cmake --build "$BUILD_DIR" --parallel "$JOBS"
    else
        cmake --build "$BUILD_DIR"
    fi
    
    print_success "Build completed successfully!"
}

# Function to build ISO image
build_iso() {
    print_status "Building ISO image..."
    
    # Create ISO directory structure
    mkdir -p "$ISO_DIR"/{boot/grub,modules}
    
    # Copy kernel and modules
    if [ -f "$BUILD_DIR/kernel/$KERNEL_BIN" ]; then
        cp "$BUILD_DIR/kernel/$KERNEL_BIN" "$ISO_DIR/boot/"
    else
        print_error "Kernel binary not found. Build the kernel first."
        return 1
    fi
    
    # Create GRUB configuration
    cat > "$ISO_DIR/boot/grub/grub.cfg" << EOF
set timeout=0
set default=0

menuentry "FG-OS" {
    multiboot2 /boot/$KERNEL_BIN
    boot
}
EOF
    
    # Create ISO image using grub-mkrescue
    if command -v grub-mkrescue >/dev/null 2>&1; then
        grub-mkrescue -o "$ISO_FILE" "$ISO_DIR"
        print_success "ISO image created: $ISO_FILE"
    else
        print_error "grub-mkrescue not found. Please install GRUB tools."
        return 1
    fi
}

# Function to run in QEMU
run_qemu() {
    print_status "Running FG-OS in QEMU..."
    
    if [ ! -f "$ISO_FILE" ]; then
        print_error "ISO file not found. Build ISO first."
        return 1
    fi
    
    # QEMU command with appropriate flags
    qemu-system-x86_64 \
        -cdrom "$ISO_FILE" \
        -m 512M \
        -enable-kvm \
        -cpu host \
        -smp 2 \
        -vga std \
        -serial stdio \
        -no-reboot \
        -no-shutdown
}

# Function to run tests
run_tests() {
    print_status "Running test suite..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Build the project first."
        return 1
    fi
    
    cd "$BUILD_DIR"
    ctest --output-on-failure
    cd ..
    
    print_success "Tests completed!"
}

# Function to display build summary
show_summary() {
    echo
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                     Build Summary                            ║${NC}"
    echo -e "${GREEN}║                                                              ║${NC}"
    echo -e "${GREEN}║  Project: $PROJECT_NAME v$PROJECT_VERSION                                   ║${NC}"
    echo -e "${GREEN}║  Build Type: $BUILD_TYPE                                         ║${NC}"
    echo -e "${GREEN}║  Target: $TARGET_ARCH                                       ║${NC}"
    echo -e "${GREEN}║  Phase: 1/20 (Research & Requirements Analysis)             ║${NC}"
    echo -e "${GREEN}║                                                              ║${NC}"
    
    if [ -f "$ISO_FILE" ]; then
        echo -e "${GREEN}║  ISO: $ISO_FILE                                      ║${NC}"
    fi
    
    echo -e "${GREEN}║                                                              ║${NC}"
    echo -e "${GREEN}║  Ready for Phase 1 development!                             ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
}

# Parse command line arguments
CLEAN=false
BUILD_ISO=false
RUN_QEMU=false
RUN_TESTS=false
JOBS=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -t|--target)
            TARGET_ARCH="$2"
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --iso)
            BUILD_ISO=true
            shift
            ;;
        --run)
            BUILD_ISO=true
            RUN_QEMU=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main build process
main() {
    print_status "Starting FG-OS build process..."
    
    # Clean if requested
    if [ "$CLEAN" = true ]; then
        clean_build
    fi
    
    # Configure build
    configure_build
    
    # Build project
    build_project
    
    # Build ISO if requested
    if [ "$BUILD_ISO" = true ]; then
        build_iso
    fi
    
    # Run tests if requested
    if [ "$RUN_TESTS" = true ]; then
        run_tests
    fi
    
    # Run in QEMU if requested
    if [ "$RUN_QEMU" = true ]; then
        run_qemu
    fi
    
    # Show summary
    show_summary
    
    print_success "Build process completed successfully!"
}

# Run main function
main "$@" 