/*
 * FG-OS Kernel Boot Header
 * Phase 3: Bootloader Development
 * 
 * Developed by: Faiz Nasir
 * Company: FGCompany Official
 * 
 * Boot-related definitions and structures for FG-OS kernel.
 */

#ifndef BOOT_H
#define BOOT_H

#include <types.h>

// Multiboot2 magic numbers
#define MULTIBOOT2_MAGIC 0x36D76289

// Boot information structure (matches kernel.h)
// Main structure is defined in kernel.h
// This file provides additional boot-specific definitions

// Boot function declarations
int parse_boot_info(uint32_t magic, void* info);
void early_boot_init(void);
void late_boot_init(void);

#endif // BOOT_H 