/* ============================================================================
 * FG-OS Kernel Linker Script
 * Phase 3: Bootloader Development
 * 
 * This linker script defines the memory layout and section organization
 * for the FG-OS kernel when loaded by GRUB or custom bootloaders.
 *
 * Author: Faiz Nasir
 * Company: FGCompany Official
 * Version: 1.0.0
 * Date: 2024
 * Copyright: © 2024 FGCompany Official. All rights reserved.
 * ============================================================================ */

/* Entry point for the kernel */
ENTRY(_start)

/* Memory layout for x86_64 */
MEMORY
{
    /* Lower memory - reserved for BIOS and bootloader */
    LOWMEM  (rwx) : ORIGIN = 0x00000000, LENGTH = 1M
    
    /* Kernel memory - where kernel is loaded (1MB+) */
    KERNEL  (rwx) : ORIGIN = 0x00100000, LENGTH = 2M
    
    /* High memory - for kernel heap and user space */
    HIGHMEM (rwx) : ORIGIN = 0x00300000, LENGTH = 1G
}

/* Define some useful constants */
KERNEL_VMA = 0xFFFFFFFF80100000;  /* Virtual memory address */
KERNEL_LMA = 0x00100000;          /* Load memory address (physical) */

/* Kernel sections */
SECTIONS
{
    /* ========================================================================
     * Multiboot Header Section
     * Must be within the first 32KB of the kernel file
     * ======================================================================== */
    
    .multiboot ALIGN(8) : AT(ADDR(.multiboot))
    {
        *(.multiboot_header)
        . = ALIGN(8);
    } >KERNEL
    
    /* ========================================================================
     * Text Section - Executable Code
     * ======================================================================== */
    
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA)
    {
        _text_start = .;
        
        /* Kernel entry point first */
        *(.text.entry)
        
        /* Main kernel code */
        *(.text .text.*)
        
        /* Constructors and destructors */
        . = ALIGN(8);
        _ctors_start = .;
        *(.ctors)
        _ctors_end = .;
        
        . = ALIGN(8);
        _dtors_start = .;
        *(.dtors)
        _dtors_end = .;
        
        _text_end = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* ========================================================================
     * Read-Only Data Section
     * ======================================================================== */
    
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        _rodata_start = .;
        
        /* Read-only strings and constants */
        *(.rodata .rodata.*)
        
        /* Exception handling tables */
        . = ALIGN(8);
        _eh_frame_start = .;
        *(.eh_frame)
        _eh_frame_end = .;
        
        _rodata_end = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* ========================================================================
     * Initialized Data Section
     * ======================================================================== */
    
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        _data_start = .;
        
        /* Initialized global and static variables */
        *(.data .data.*)
        
        /* Small data sections */
        *(.sdata .sdata.*)
        
        _data_end = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* ========================================================================
     * Uninitialized Data Section (BSS)
     * ======================================================================== */
    
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        _bss_start = .;
        
        /* Uninitialized global and static variables */
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        
        _bss_end = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* ========================================================================
     * Kernel Stack Section
     * ======================================================================== */
    
    .stack ALIGN(4K) : AT(ADDR(.stack) - KERNEL_VMA + KERNEL_LMA)
    {
        _stack_bottom = .;
        . += 0x4000;  /* 16KB stack */
        _stack_top = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* ========================================================================
     * Debug Information Sections
     * These sections are not loaded into memory but provide debugging info
     * ======================================================================== */
    
    /* DWARF debug sections */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    .debug_ranges   0 : { *(.debug_ranges) }
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    
    /* Symbol table sections */
    .symtab         0 : { *(.symtab) }
    .strtab         0 : { *(.strtab) }
    .shstrtab       0 : { *(.shstrtab) }
    
    /* ========================================================================
     * Phase 3 Specific Sections
     * ======================================================================== */
    
    /* Bootloader interface data */
    .boot_data ALIGN(4K) : AT(ADDR(.boot_data) - KERNEL_VMA + KERNEL_LMA)
    {
        _boot_data_start = .;
        *(.boot_data .boot_data.*)
        _boot_data_end = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* Hardware detection and initialization */
    .hw_init ALIGN(4K) : AT(ADDR(.hw_init) - KERNEL_VMA + KERNEL_LMA)
    {
        _hw_init_start = .;
        *(.hw_init .hw_init.*)
        _hw_init_end = .;
        . = ALIGN(4K);
    } >KERNEL
    
    /* ========================================================================
     * Memory Layout Symbols
     * ======================================================================== */
    
    _kernel_start = KERNEL_LMA;
    _kernel_end = .;
    _kernel_size = _kernel_end - _kernel_start;
    
    /* Virtual memory addresses */
    _kernel_virtual_start = KERNEL_VMA;
    _kernel_virtual_end = KERNEL_VMA + _kernel_size;
    
    /* Physical memory addresses */
    _kernel_physical_start = KERNEL_LMA;
    _kernel_physical_end = KERNEL_LMA + _kernel_size;
    
    /* Section sizes */
    _text_size = _text_end - _text_start;
    _rodata_size = _rodata_end - _rodata_start;
    _data_size = _data_end - _data_start;
    _bss_size = _bss_end - _bss_start;
    
    /* ========================================================================
     * Discard Unwanted Sections
     * ======================================================================== */
    
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.note.*)
        *(.gnu.hash)
        *(.dynsym)
        *(.dynstr)
        *(.dynamic)
        *(.hash)
        *(.interp)
        *(.got)
        *(.got.plt)
        *(.plt)
        *(.rel.*)
        *(.rela.*)
    }
}

/* ============================================================================
 * Assertions and Validation
 * ============================================================================ */

/* Ensure kernel doesn't exceed memory bounds */
ASSERT(_kernel_size < 2M, "Kernel size exceeds 2MB limit")

/* Ensure proper alignment */
ASSERT((_text_start % 4096) == 0, "Text section not page-aligned")
ASSERT((_rodata_start % 4096) == 0, "Rodata section not page-aligned")
ASSERT((_data_start % 4096) == 0, "Data section not page-aligned")
ASSERT((_bss_start % 4096) == 0, "BSS section not page-aligned")

/* Ensure sections don't overlap */
ASSERT(_text_end <= _rodata_start, "Text and rodata sections overlap")
ASSERT(_rodata_end <= _data_start, "Rodata and data sections overlap")
ASSERT(_data_end <= _bss_start, "Data and BSS sections overlap")

/* Validate multiboot header position */
ASSERT(SIZEOF(.multiboot) > 0, "Multiboot header section is empty")
ASSERT(ADDR(.multiboot) < (KERNEL_LMA + 32K), "Multiboot header beyond 32KB limit")

/* ============================================================================
 * Phase 3 Bootloader Integration Points
 * ============================================================================ */

/* These symbols are used by the bootloader to locate kernel components */
PROVIDE(_multiboot_header = ADDR(.multiboot));
PROVIDE(_kernel_entry = _text_start);
PROVIDE(_kernel_stack = _stack_top);

/* Memory management symbols for early initialization */
PROVIDE(_available_memory_start = _kernel_end);
PROVIDE(_heap_start = _bss_end);

/* Debug symbols for development */
PROVIDE(_debug_info_start = ADDR(.debug_info));
PROVIDE(_symbol_table_start = ADDR(.symtab));

/* ============================================================================
 * FG-OS Specific Metadata
 * ============================================================================ */

/* Kernel metadata for runtime introspection */
PROVIDE(_fgos_version = 0x00010000);     /* Version 1.0.0 */
PROVIDE(_fgos_phase = 3);                /* Current phase */
PROVIDE(_fgos_architecture = 0x64);      /* x86_64 */
PROVIDE(_build_timestamp = 0);           /* Build timestamp (filled by build system) */

/* ============================================================================
 * End of Linker Script
 * ============================================================================ */ 