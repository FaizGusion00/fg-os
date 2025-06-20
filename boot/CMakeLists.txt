# FG-OS Bootloader Build Configuration
# Phase 3: Bootloader Development
# Developed by: Faiz Nasir - FGCompany Official

cmake_minimum_required(VERSION 3.20)

# Bootloader configuration
set(BOOTLOADER_NAME "fg-boot")
set(BOOTLOADER_VERSION "1.0.0")

# Cross-compilation setup for x86_64
if(CMAKE_CROSSCOMPILING)
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
endif()

# Bootloader specific compiler flags
set(BOOTLOADER_C_FLAGS 
    "-ffreestanding"
    "-fno-stack-protector" 
    "-fno-pic"
    "-mno-80387"
    "-mno-mmx"
    "-mno-sse"
    "-mno-sse2"
    "-mno-red-zone"
    "-mcmodel=kernel"
    "-Wall"
    "-Wextra"
    "-std=c11"
)

set(BOOTLOADER_ASM_FLAGS
    "-f elf64"
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel/include
)

# Bootloader source files
set(BOOTLOADER_SOURCES
    # Assembly entry points
    src/boot.asm
    src/multiboot.asm
    src/gdt.asm
    src/idt.asm
    
    # C source files
    src/main.c
    src/console.c
    src/memory.c
    src/hardware.c
    src/loader.c
    src/grub.c
)

# GRUB configuration files
set(GRUB_CONFIG_FILES
    grub/grub.cfg
    grub/menu.lst
)

# Multiboot header
set(MULTIBOOT_SOURCES
    src/multiboot_header.asm
)

# Create bootloader executable
if(CMAKE_ASM_NASM_COMPILER)
    enable_language(ASM_NASM)
    
    # Compile assembly files
    foreach(ASM_FILE ${MULTIBOOT_SOURCES})
        get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)
        set(ASM_OBJ "${CMAKE_CURRENT_BINARY_DIR}/${ASM_NAME}.o")
        
        add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND ${CMAKE_ASM_NASM_COMPILER} ${BOOTLOADER_ASM_FLAGS} 
                    -o ${ASM_OBJ} ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_FILE}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_FILE}
            COMMENT "Assembling ${ASM_FILE}"
        )
        
        list(APPEND BOOTLOADER_OBJECTS ${ASM_OBJ})
    endforeach()
    
    # Create bootloader binary
    add_custom_target(bootloader
        DEPENDS ${BOOTLOADER_OBJECTS}
        COMMENT "Building FG-OS bootloader"
    )
else()
    message(WARNING "NASM not found - bootloader assembly will be skipped in Phase 3")
    
    # Create placeholder bootloader target
    add_custom_target(bootloader
        COMMAND ${CMAKE_COMMAND} -E echo "Phase 3: Bootloader framework ready"
        COMMENT "Bootloader target (NASM required for full implementation)"
    )
endif()

# GRUB integration
add_custom_target(grub-config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/grub
            ${CMAKE_BINARY_DIR}/iso/boot/grub
    COMMENT "Copying GRUB configuration"
)

# ISO generation target
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fg-kernel> ${CMAKE_BINARY_DIR}/iso/boot/
    DEPENDS fg-kernel grub-config
    COMMENT "Creating bootable ISO structure"
)

# Enhanced ISO with GRUB (if grub-mkrescue is available)
find_program(GRUB_MKRESCUE grub-mkrescue)
if(GRUB_MKRESCUE)
    add_custom_target(bootable-iso
        COMMAND ${GRUB_MKRESCUE} -o ${CMAKE_BINARY_DIR}/fg-os.iso ${CMAKE_BINARY_DIR}/iso
        DEPENDS iso
        COMMENT "Creating bootable FG-OS ISO with GRUB"
    )
else()
    add_custom_target(bootable-iso
        COMMAND ${CMAKE_COMMAND} -E echo "grub-mkrescue not found - install GRUB tools"
        COMMENT "Bootable ISO target (requires GRUB tools)"
    )
endif()

# Boot testing targets
find_program(QEMU_X86_64 qemu-system-x86_64)
if(QEMU_X86_64)
    # Test with QEMU
    add_custom_target(test-boot
        COMMAND ${QEMU_X86_64} 
                -cdrom ${CMAKE_BINARY_DIR}/fg-os.iso
                -m 512M
                -boot d
                -serial stdio
                -monitor none
        DEPENDS bootable-iso
        COMMENT "Testing FG-OS boot in QEMU"
    )
    
    # Debug boot with GDB
    add_custom_target(debug-boot
        COMMAND ${QEMU_X86_64}
                -cdrom ${CMAKE_BINARY_DIR}/fg-os.iso
                -m 512M
                -boot d
                -serial stdio
                -s -S
        DEPENDS bootable-iso
        COMMENT "Debug FG-OS boot with GDB (use 'target remote :1234' in GDB)"
    )
else()
    add_custom_target(test-boot
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU not found - install qemu-system-x86"
        COMMENT "Boot test target (requires QEMU)"
    )
    
    add_custom_target(debug-boot
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU not found - install qemu-system-x86"
        COMMENT "Boot debug target (requires QEMU)"
    )
endif()

# Bootloader installation targets
add_custom_target(install-bootloader
    COMMAND ${CMAKE_COMMAND} -E echo "Bootloader installation will be implemented in Phase 4"
    COMMENT "Install bootloader to disk (Phase 4 feature)"
)

# Phase 3 validation
add_custom_target(phase3-validation
    COMMAND ${CMAKE_COMMAND} -E echo "=== Phase 3 Validation ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Bootloader: ${BOOTLOADER_NAME} v${BOOTLOADER_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "NASM: ${CMAKE_ASM_NASM_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo "GRUB: ${GRUB_MKRESCUE}"
    COMMAND ${CMAKE_COMMAND} -E echo "QEMU: ${QEMU_X86_64}"
    COMMENT "Validating Phase 3 bootloader development environment"
)

# Print Phase 3 configuration
message(STATUS "=== Phase 3: Bootloader Configuration ===")
message(STATUS "Bootloader: ${BOOTLOADER_NAME} v${BOOTLOADER_VERSION}")
message(STATUS "NASM Assembler: ${CMAKE_ASM_NASM_COMPILER}")
message(STATUS "GRUB Tools: ${GRUB_MKRESCUE}")
message(STATUS "QEMU Emulator: ${QEMU_X86_64}")
message(STATUS "Cross-compilation: ${CMAKE_CROSSCOMPILING}")
message(STATUS "Target Architecture: x86_64")
message(STATUS "==========================================")

# Add bootloader to main build
add_dependencies(iso bootloader)

# Install targets
install(FILES ${CMAKE_BINARY_DIR}/fg-os.iso 
        DESTINATION bin 
        OPTIONAL)

install(DIRECTORY grub/
        DESTINATION share/fg-os/grub
        FILES_MATCHING PATTERN "*.cfg" PATTERN "*.lst") 