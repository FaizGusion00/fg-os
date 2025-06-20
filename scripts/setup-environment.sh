#!/bin/bash

# FG-OS Development Environment Setup Script
# Developed by: Faiz Nasir - FGCompany Official
# Description: Sets up the complete development environment for FG-OS

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
PROJECT_AUTHOR="Faiz Nasir"
PROJECT_COMPANY="FGCompany Official"

# Print header
echo -e "${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║                    FG-OS Development Setup                   ║${NC}"
echo -e "${BLUE}║                                                              ║${NC}"
echo -e "${BLUE}║  Version: ${PROJECT_VERSION}                                         ║${NC}"
echo -e "${BLUE}║  Author:  ${PROJECT_AUTHOR}                                    ║${NC}"
echo -e "${BLUE}║  Company: ${PROJECT_COMPANY}                              ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
echo

# Function to print status messages
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to create directory structure
create_directories() {
    print_status "Creating project directory structure..."
    
    # Main directories
    mkdir -p bootloader/{grub,custom}
    mkdir -p kernel/{arch/x86_64,mm,sched,fs,net,drivers,ipc}
    mkdir -p kernel/include/{arch,mm,sched,fs,net,drivers,ipc}
    mkdir -p graphics/{rendering,compositor,themes,fonts,input}
    mkdir -p compat/{linux/{posix,elf,syscalls},windows/{win32,pe,registry},native}
    mkdir -p services/{desktop,filemanager,terminal,settings,packagemgr}
    mkdir -p libs/{libc,libcxx,libfg}
    mkdir -p tools/{compiler,debugger,profiler}
    mkdir -p tests/{unit,integration,system}
    mkdir -p docs/{api,developer,user,architecture}
    mkdir -p scripts/{build,deploy,test}
    mkdir -p assets/{themes,fonts,icons,wallpapers}
    mkdir -p config
    mkdir -p build
    mkdir -p iso
    
    # Include directories
    mkdir -p include/{kernel,graphics,compat,services,libs}
    
    print_success "Directory structure created successfully!"
}

# Function to install dependencies on Ubuntu/Debian
install_ubuntu_deps() {
    print_status "Installing dependencies for Ubuntu/Debian..."
    
    sudo apt update
    sudo apt install -y \
        build-essential \
        cmake \
        gcc-multilib \
        g++-multilib \
        nasm \
        mtools \
        xorriso \
        grub-pc-bin \
        grub-common \
        qemu-system-x86 \
        qemu-utils \
        gdb \
        git \
        curl \
        wget \
        python3 \
        python3-pip \
        doxygen \
        graphviz \
        clang \
        clang-format \
        valgrind \
        bochs \
        bochs-x \
        libsdl2-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev
        
    print_success "Ubuntu/Debian dependencies installed!"
}

# Function to install dependencies on Fedora/RHEL
install_fedora_deps() {
    print_status "Installing dependencies for Fedora/RHEL..."
    
    sudo dnf install -y \
        gcc \
        gcc-c++ \
        cmake \
        nasm \
        mtools \
        xorriso \
        grub2-tools \
        qemu-system-x86 \
        qemu-img \
        gdb \
        git \
        curl \
        wget \
        python3 \
        python3-pip \
        doxygen \
        graphviz \
        clang \
        clang-tools-extra \
        valgrind \
        bochs \
        SDL2-devel \
        mesa-libGL-devel \
        mesa-libGLU-devel
        
    print_success "Fedora/RHEL dependencies installed!"
}

# Function to install dependencies on macOS
install_macos_deps() {
    print_status "Installing dependencies for macOS..."
    
    # Check if Homebrew is installed
    if ! command_exists brew; then
        print_warning "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    brew update
    brew install \
        cmake \
        nasm \
        mtools \
        xorriso \
        qemu \
        gdb \
        git \
        curl \
        wget \
        python3 \
        doxygen \
        graphviz \
        llvm \
        clang-format \
        sdl2
        
    print_success "macOS dependencies installed!"
}

# Function to setup cross-compiler
setup_cross_compiler() {
    print_status "Setting up cross-compiler toolchain..."
    
    local TARGET="x86_64-elf"
    local PREFIX="$HOME/opt/cross"
    local BINUTILS_VERSION="2.40"
    local GCC_VERSION="13.2.0"
    
    # Create directories
    mkdir -p "$PREFIX"
    mkdir -p "$HOME/src"
    
    # Download and build binutils
    if [ ! -f "$PREFIX/bin/$TARGET-ld" ]; then
        print_status "Building binutils..."
        cd "$HOME/src"
        wget -nc "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"
        tar -xf "binutils-$BINUTILS_VERSION.tar.gz"
        mkdir -p "build-binutils"
        cd "build-binutils"
        "../binutils-$BINUTILS_VERSION/configure" --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
        make -j$(nproc)
        make install
        cd ..
    fi
    
    # Download and build GCC
    if [ ! -f "$PREFIX/bin/$TARGET-gcc" ]; then
        print_status "Building GCC..."
        cd "$HOME/src"
        wget -nc "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"
        tar -xf "gcc-$GCC_VERSION.tar.gz"
        cd "gcc-$GCC_VERSION"
        contrib/download_prerequisites
        cd ..
        mkdir -p "build-gcc"
        cd "build-gcc"
        "../gcc-$GCC_VERSION/configure" --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
        make -j$(nproc) all-gcc
        make -j$(nproc) all-target-libgcc
        make install-gcc
        make install-target-libgcc
        cd ..
    fi
    
    # Add to PATH
    echo "export PATH=\"$PREFIX/bin:\$PATH\"" >> ~/.bashrc
    export PATH="$PREFIX/bin:$PATH"
    
    print_success "Cross-compiler toolchain setup complete!"
}

# Function to setup Git configuration
setup_git() {
    print_status "Setting up Git configuration..."
    
    # Initialize git repository if not already done
    if [ ! -d ".git" ]; then
        git init
        echo "# FG-OS Build Artifacts" > .gitignore
        echo "build/" >> .gitignore
        echo "iso/" >> .gitignore
        echo "*.o" >> .gitignore
        echo "*.bin" >> .gitignore
        echo "*.img" >> .gitignore
        echo "*.iso" >> .gitignore
        echo ".vscode/" >> .gitignore
        echo "compile_commands.json" >> .gitignore
        
        git add .
        git commit -m "Initial FG-OS project setup by $PROJECT_AUTHOR"
    fi
    
    print_success "Git configuration complete!"
}

# Function to create configuration files
create_config_files() {
    print_status "Creating configuration files..."
    
    # Create .editorconfig
    cat > .editorconfig << EOF
root = true

[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true

[*.{c,h,cpp,hpp,cc,hh}]
indent_style = space
indent_size = 4

[*.{s,S,asm}]
indent_style = space
indent_size = 8

[*.{cmake,CMakeLists.txt}]
indent_style = space
indent_size = 4

[*.{md,txt}]
trim_trailing_whitespace = false

[Makefile]
indent_style = tab
EOF

    # Create clang-format configuration
    cat > .clang-format << EOF
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlignTrailingComments: true
SpaceAfterCStyleCast: true
PointerAlignment: Right
AlignConsecutiveDeclarations: true
EOF

    print_success "Configuration files created!"
}

# Function to detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command_exists apt; then
            OS="ubuntu"
        elif command_exists dnf; then
            OS="fedora"
        else
            OS="linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
    else
        OS="unknown"
    fi
}

# Main setup function
main() {
    print_status "Starting FG-OS development environment setup..."
    
    # Detect operating system
    detect_os
    print_status "Detected OS: $OS"
    
    # Create directory structure
    create_directories
    
    # Install dependencies based on OS
    case $OS in
        ubuntu)
            install_ubuntu_deps
            ;;
        fedora)
            install_fedora_deps
            ;;
        macos)
            install_macos_deps
            ;;
        windows)
            print_warning "Windows detected. Please use WSL or install dependencies manually."
            ;;
        *)
            print_warning "Unknown OS. Please install dependencies manually."
            ;;
    esac
    
    # Setup cross-compiler (skip on Windows for now)
    if [[ "$OS" != "windows" ]]; then
        read -p "Do you want to setup cross-compiler toolchain? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            setup_cross_compiler
        fi
    fi
    
    # Setup Git
    setup_git
    
    # Create configuration files
    create_config_files
    
    print_success "FG-OS development environment setup complete!"
    print_status "You can now run 'cmake -B build && cmake --build build' to start building."
    print_status "Phase 1 (Research & Requirements Analysis) is ready to begin!"
    
    echo
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                     Setup Complete!                         ║${NC}"
    echo -e "${GREEN}║                                                              ║${NC}"
    echo -e "${GREEN}║  Next steps:                                                 ║${NC}"
    echo -e "${GREEN}║  1. Review docs/ARCHITECTURE.md                             ║${NC}"
    echo -e "${GREEN}║  2. Run ./scripts/build.sh to build the project             ║${NC}"
    echo -e "${GREEN}║  3. Start with Phase 1 development tasks                    ║${NC}"
    echo -e "${GREEN}║                                                              ║${NC}"
    echo -e "${GREEN}║  Happy coding! - ${PROJECT_AUTHOR}                         ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
}

# Run main function
main "$@" 