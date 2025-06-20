cmake_minimum_required(VERSION 3.20)

# FG-OS Project Configuration
project(FG-OS 
    VERSION 1.0.0
    DESCRIPTION "Advanced Cross-Platform Operating System"
    LANGUAGES C CXX ASM
)

# Project Information
set(PROJECT_AUTHOR "Faiz Nasir")
set(PROJECT_COMPANY "FGCompany Official")
set(PROJECT_COPYRIGHT "© 2024 FGCompany Official. All rights reserved.")

# Build Configuration
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_KERNEL "Build FG-OS Kernel" ON)
option(BUILD_BOOTLOADER "Build FG-OS Bootloader" ON)
option(BUILD_GRAPHICS "Build Graphics Subsystem" ON)
option(BUILD_COMPAT_LAYER "Build Compatibility Layer" ON)
option(BUILD_SERVICES "Build System Services" ON)
option(BUILD_TESTS "Build Test Suite" ON)
option(BUILD_DOCUMENTATION "Build Documentation" ON)

# Target Architecture
if(NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "x86_64")
endif()

# Compiler Flags
set(KERNEL_CFLAGS "-ffreestanding -fno-stack-protector -fno-builtin -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -O2")

set(USER_CFLAGS "-Wall -Wextra -Werror -O2 -fPIC")

# Include Directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/libs/include
)

# Subdirectories
if(BUILD_BOOTLOADER)
    add_subdirectory(boot)
endif()

if(BUILD_KERNEL)
    add_subdirectory(kernel)
endif()

if(BUILD_GRAPHICS AND EXISTS ${CMAKE_SOURCE_DIR}/graphics/CMakeLists.txt)
    add_subdirectory(graphics)
endif()

# Compatibility layer deferred to Phase 15
# if(BUILD_COMPAT_LAYER)
#     add_subdirectory(compat)
# endif()

# System services deferred to Phase 17
# if(BUILD_SERVICES)
#     add_subdirectory(services)
# endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/libs/CMakeLists.txt)
    add_subdirectory(libs)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/tools/CMakeLists.txt)
    add_subdirectory(tools)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation build deferred until needed
# if(BUILD_DOCUMENTATION)
#     add_subdirectory(docs)
# endif()

# Custom Targets
add_custom_target(fgos-iso
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/build-iso.sh
    DEPENDS kernel
    COMMENT "Building FG-OS ISO image"
)

add_custom_target(run-qemu
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/run-qemu.sh
    DEPENDS fgos-iso
    COMMENT "Running FG-OS in QEMU"
)

add_custom_target(debug-qemu
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/debug-qemu.sh
    DEPENDS fgos-iso
    COMMENT "Running FG-OS in QEMU with debugging"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build artifacts"
)

# Print Configuration Summary
message(STATUS "=== FG-OS Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Author: ${PROJECT_AUTHOR}")
message(STATUS "Company: ${PROJECT_COMPANY}")
message(STATUS "Target Architecture: ${TARGET_ARCH}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "  Kernel: ${BUILD_KERNEL}")
message(STATUS "  Bootloader: ${BUILD_BOOTLOADER}")
message(STATUS "  Graphics: ${BUILD_GRAPHICS}")
message(STATUS "  Compatibility Layer: ${BUILD_COMPAT_LAYER}")
message(STATUS "  System Services: ${BUILD_SERVICES}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Documentation: ${BUILD_DOCUMENTATION}")
message(STATUS "===================================")

# Development Phase Tracking
set(CURRENT_PHASE "11" CACHE STRING "Current development phase (1-20)")
message(STATUS "Current Development Phase: ${CURRENT_PHASE}/20")

# Phase-specific configurations
if(CURRENT_PHASE EQUAL 1)
    message(STATUS "Phase 1: Research & Requirements Analysis")
    message(STATUS "- Setting up development environment")
    message(STATUS "- Defining architecture and specifications")
elseif(CURRENT_PHASE EQUAL 2)
    message(STATUS "Phase 2: Development Environment Setup")
    message(STATUS "- Configuring build system")
    message(STATUS "- Setting up testing framework")
    message(STATUS "- Cross-compiler toolchain setup")
    message(STATUS "- CI/CD pipeline configuration")
elseif(CURRENT_PHASE EQUAL 3)
    message(STATUS "Phase 3: Bootloader Development")
    message(STATUS "- GRUB configuration and multiboot support")
    message(STATUS "- Custom bootloader implementation")
    message(STATUS "- Hardware detection and initialization")
    message(STATUS "- Boot sequence optimization")
elseif(CURRENT_PHASE EQUAL 4)
    message(STATUS "Phase 4: Kernel Architecture Design")
    message(STATUS "- Memory management architecture")
    message(STATUS "- Process and thread management design")
    message(STATUS "- Interrupt and exception handling")
    message(STATUS "- System call interface design")
    message(STATUS "- Security framework architecture")
elseif(CURRENT_PHASE EQUAL 5)
    message(STATUS "Phase 5: Memory Management Implementation")
    message(STATUS "- Physical memory manager implementation")
    message(STATUS "- Virtual memory manager implementation")
    message(STATUS "- Kernel heap allocator implementation")
    message(STATUS "- Page fault handling")
    message(STATUS "- Memory protection implementation")
elseif(CURRENT_PHASE EQUAL 6)
    message(STATUS "Phase 6: Process Management System")
    message(STATUS "- Process creation and destruction")
    message(STATUS "- Thread management implementation")
    message(STATUS "- Scheduler implementation")
    message(STATUS "- Context switching framework")
    message(STATUS "- Process control and synchronization")
elseif(CURRENT_PHASE EQUAL 7)
    message(STATUS "Phase 7: Interrupt Handling System")
    message(STATUS "- Interrupt descriptor table setup")
    message(STATUS "- Hardware interrupt handlers")
    message(STATUS "- Software interrupt implementation")
    message(STATUS "- Timer and clock management")
    message(STATUS "- Exception handling")
elseif(CURRENT_PHASE EQUAL 8)
    message(STATUS "Phase 8: Device Drivers Framework")
    message(STATUS "- Universal device management system")
    message(STATUS "- PCI bus enumeration and configuration")
    message(STATUS "- Driver registration framework")
    message(STATUS "- Hardware abstraction layer")
    message(STATUS "- Storage and input device drivers")
elseif(CURRENT_PHASE EQUAL 9)
    message(STATUS "Phase 9: Hardware Abstraction Layer (HAL)")
    message(STATUS "- Platform-specific code abstraction")
    message(STATUS "- Hardware detection and enumeration")
    message(STATUS "- Cross-platform compatibility interface")
    message(STATUS "- System resource management")
    message(STATUS "- Integration testing framework")
elseif(CURRENT_PHASE EQUAL 10)
    message(STATUS "Phase 10: File System Design")
    message(STATUS "- File system architecture design")
    message(STATUS "- Directory structure specification")
    message(STATUS "- File permissions and security model")
    message(STATUS "- Journaling system design")
    message(STATUS "- Cross-platform file format support")
elseif(CURRENT_PHASE EQUAL 11)
    message(STATUS "Phase 11: File System Implementation")
    message(STATUS "- Native file system implementation")
    message(STATUS "- FAT32/NTFS compatibility layer")
    message(STATUS "- ext4/ext3 compatibility layer")
    message(STATUS "- File caching and optimization")
    message(STATUS "- File system utilities")
endif() 