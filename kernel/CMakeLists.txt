# FG-OS Kernel CMakeLists.txt
# Developed by: Faiz Nasir - FGCompany Official

cmake_minimum_required(VERSION 3.20)

# Kernel configuration
set(KERNEL_NAME "fg-kernel")
set(KERNEL_VERSION "1.0.0")

# Kernel-specific compiler flags
set(CMAKE_C_FLAGS "${KERNEL_CFLAGS}")
set(CMAKE_CXX_FLAGS "${KERNEL_CFLAGS}")

# Phase 7: Interrupt Handling System
set(KERNEL_SOURCES
    # Core kernel files
    src/main.c
    src/boot.c
    src/panic.c
    src/console_stub.c
    src/string_stubs.c
    
    # Phase 5: Memory management implementation
    mm/pmm.c
    mm/vmm.c
    mm/heap.c
    mm/memory_utils.c
    
    # Phase 6: Process management implementation
    sched/process.c
    sched/thread.c
    sched/scheduler.c
    
    # Phase 7: Interrupt handling implementation
    interrupt/idt.c
    interrupt/interrupt.c
    
    # Phase 5: Architecture stubs
    arch/x86_64/arch_stubs.c
    
    # Additional files will be added in later phases:
    # Phase 8: Device drivers (drivers/*)
    # Phase 9: File system (fs/*)
    # Phase 10: Inter-process communication (ipc/*)
)

# Phase 3: Essential kernel headers only
set(KERNEL_HEADERS
    include/kernel.h
    include/types.h
    include/boot.h
    include/panic.h
    # Additional headers will be added in later phases
)

# Linker script
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/${TARGET_ARCH}/include
)

# Create kernel executable
add_executable(${KERNEL_NAME} ${KERNEL_SOURCES})

# Create Phase 7 demonstration executable
add_executable(phase7-demo src/test_main.c)

# Set kernel properties (Phase 3: Simplified for Windows build)
set_target_properties(${KERNEL_NAME} PROPERTIES
    COMPILE_FLAGS "${KERNEL_CFLAGS}"
)

# Custom target to create kernel binary (Phase 3: Deferred)
# add_custom_command(TARGET ${KERNEL_NAME} POST_BUILD
#     COMMAND ${CMAKE_OBJCOPY} -O binary ${KERNEL_NAME} kernel.bin
#     COMMENT "Creating kernel binary"
# )

# Install kernel
install(TARGETS ${KERNEL_NAME} DESTINATION boot)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin DESTINATION boot)

# Phase-specific targets
if(CURRENT_PHASE EQUAL 1)
    message(STATUS "Building Phase 1 kernel components...")
    
    # Create minimal kernel for Phase 1
    add_custom_target(phase1-kernel
        DEPENDS ${KERNEL_NAME}
        COMMENT "Phase 1: Basic kernel structure ready"
    )
elseif(CURRENT_PHASE EQUAL 7)
    message(STATUS "Building Phase 7 kernel components...")
    
    # Create interrupt handling kernel for Phase 7
    add_custom_target(phase7-kernel
        DEPENDS ${KERNEL_NAME}
        COMMENT "Phase 7: Interrupt handling system ready"
    )
    
    add_custom_target(phase7-test
        DEPENDS phase7-demo
        COMMENT "Phase 7: Interrupt handling demonstration ready"
    )
endif()

# Print kernel configuration
message(STATUS "=== Kernel Configuration ===")
message(STATUS "Kernel Name: ${KERNEL_NAME}")
message(STATUS "Kernel Version: ${KERNEL_VERSION}")
message(STATUS "Target Architecture: ${TARGET_ARCH}")
message(STATUS "Linker Script: ${LINKER_SCRIPT}")
message(STATUS "==============================") 