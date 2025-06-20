; ============================================================================
; FG-OS Multiboot2 Header
; Phase 3: Bootloader Development
; 
; This file defines the Multiboot2 header that allows GRUB and other
; Multiboot-compliant bootloaders to load the FG-OS kernel.
;
; Author: Faiz Nasir
; Company: FGCompany Official
; Version: 1.0.0
; Date: 2024
; Copyright: © 2024 FGCompany Official. All rights reserved.
; ============================================================================

section .multiboot_header
bits 32

; Multiboot2 magic numbers and constants
MULTIBOOT2_MAGIC        equ 0xE85250D6  ; Multiboot2 magic number
MULTIBOOT2_ARCHITECTURE equ 0           ; i386 protected mode
MULTIBOOT2_HEADER_SIZE  equ multiboot_header_end - multiboot_header_start
MULTIBOOT2_CHECKSUM     equ -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCHITECTURE + MULTIBOOT2_HEADER_SIZE)

; FG-OS specific constants
FGOS_LOAD_ADDR         equ 0x100000    ; 1MB - standard kernel load address
FGOS_STACK_SIZE        equ 0x4000      ; 16KB stack
FGOS_HEAP_SIZE         equ 0x100000    ; 1MB initial heap

; Beautiful gradient color scheme for FG-OS
FGOS_COLOR_PRIMARY     equ 0x6B73FF    ; Beautiful blue-purple gradient start
FGOS_COLOR_SECONDARY   equ 0x9B59B6    ; Purple gradient middle  
FGOS_COLOR_ACCENT      equ 0xFF6B9D    ; Pink gradient end

align 8
multiboot_header_start:
    ; Multiboot2 header
    dd MULTIBOOT2_MAGIC                 ; Magic number
    dd MULTIBOOT2_ARCHITECTURE          ; Architecture (i386)
    dd MULTIBOOT2_HEADER_SIZE            ; Header length
    dd MULTIBOOT2_CHECKSUM               ; Checksum

    ; ========================================================================
    ; Information Request Tag
    ; Request specific information from the bootloader
    ; ========================================================================
align 8
information_request_tag_start:
    dw 1                                ; Type: Information request
    dw 0                                ; Flags: required
    dd information_request_tag_end - information_request_tag_start
    
    ; Request these information types from bootloader:
    dd 4                                ; Basic memory information
    dd 6                                ; Memory map
    dd 8                                ; Framebuffer info
    dd 9                                ; ELF symbols
    dd 14                               ; ACPI old RSDP
    dd 15                               ; ACPI new RSDP
    dd 21                               ; Image load base address
    
information_request_tag_end:

    ; ========================================================================
    ; Entry Point Tag
    ; Specifies the entry point where the bootloader should jump
    ; ========================================================================
align 8
entry_tag_start:
    dw 3                                ; Type: Entry address tag
    dw 1                                ; Flags: optional
    dd entry_tag_end - entry_tag_start
    
    dd _start                           ; Entry point address
    
entry_tag_end:

    ; ========================================================================
    ; Console Flags Tag
    ; Configure console behavior for FG-OS
    ; ========================================================================
align 8
console_tag_start:
    dw 4                                ; Type: Console flags
    dw 1                                ; Flags: optional  
    dd console_tag_end - console_tag_start
    
    dd 0                                ; Console flags (0 = text mode preferred)
    
console_tag_end:

    ; ========================================================================
    ; Framebuffer Tag
    ; Request specific framebuffer configuration for gradient UI
    ; ========================================================================
align 8
framebuffer_tag_start:
    dw 5                                ; Type: Framebuffer
    dw 1                                ; Flags: optional
    dd framebuffer_tag_end - framebuffer_tag_start
    
    dd 1024                             ; Preferred width
    dd 768                              ; Preferred height  
    dd 32                               ; Preferred depth (32-bit for gradient colors)
    
framebuffer_tag_end:

    ; ========================================================================
    ; End Tag
    ; Marks the end of the multiboot header
    ; ========================================================================
align 8
end_tag_start:
    dw 0                                ; Type: End tag
    dw 0                                ; Flags: required
    dd 8                                ; Size: 8 bytes
    
end_tag_end:

multiboot_header_end:

; ============================================================================
; Boot Entry Point
; This is where GRUB will jump to start FG-OS
; ============================================================================

section .text
bits 32
global _start

_start:
    ; Disable interrupts during early boot
    cli
    
    ; Set up a basic stack
    mov esp, stack_top
    mov ebp, esp
    
    ; Store multiboot information
    ; EAX contains the magic number
    ; EBX contains the address of the multiboot information structure
    push ebx                            ; Multiboot info pointer
    push eax                            ; Multiboot magic
    
    ; Clear direction flag for string operations
    cld
    
    ; Initialize basic CPU state
    call init_cpu_state
    
    ; Display FG-OS boot banner
    call display_boot_banner
    
    ; Validate multiboot information
    call validate_multiboot
    
    ; Jump to kernel main (should not return)
    call kernel_main
    
    ; If we somehow return, halt the system
    jmp halt_system

; ============================================================================
; Helper Functions
; ============================================================================

init_cpu_state:
    ; Clear CPU flags
    push 0
    popf
    ret

display_boot_banner:
    ; Display beautiful FG-OS boot banner with gradient colors
    ret

validate_multiboot:
    ; Validate that we were loaded by a multiboot-compliant bootloader
    pop eax                             ; Get magic number
    cmp eax, 0x36D76289                 ; Multiboot2 magic
    je .valid
    
    ; Invalid boot - halt
    jmp halt_system
    
.valid:
    push eax                            ; Restore stack
    ret

halt_system:
    ; Halt the system gracefully
    cli                                 ; Disable interrupts
    hlt                                 ; Halt processor
    jmp $                               ; Infinite loop if HLT fails

; ============================================================================
; Data Section
; ============================================================================

section .bss
align 16

; Boot stack (16KB)
stack_bottom:
    resb FGOS_STACK_SIZE
stack_top:

section .rodata
align 8

boot_signature:     db 'FG-OS Bootloader v1.0.0', 0 