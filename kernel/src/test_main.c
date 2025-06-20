/**
 * @file test_main.c
 * @brief Test main function for FG-OS kernel demonstrations
 * 
 * This file provides the main entry point for testing various kernel
 * subsystems and demonstrating FG-OS capabilities for different phases.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright Copyright (c) 2024 FGCompany Official. All rights reserved.
 */

#include "../include/kernel.h"

// External test function declarations
extern void test_phase9(void);
extern void test_phase10(void);
extern void test_phase11(void);

/**
 * @brief Clear the console screen
 */
void clear_screen(void) {
    printf("\033[2J\033[H");
}

/**
 * @brief Main test function
 * 
 * Entry point for kernel testing and demonstration programs.
 * This function determines which phase test to run based on the
 * current kernel configuration.
 */
int main(void) {
    // Determine which phase test to run
    if (CURRENT_PHASE == 9) {
        printf("Starting FG-OS Phase 9 Hardware Abstraction Layer Test...\n\n");
        test_phase9();
    } else if (CURRENT_PHASE == 10) {
        printf("Starting FG-OS Phase 10 File System Design Test...\n\n");
        test_phase10();
    } else if (CURRENT_PHASE == 11) {
        printf("Starting FG-OS Phase 11 File System Implementation Test...\n\n");
        test_phase11();
    } else {
        printf("FG-OS Phase %d Test - Not yet implemented\n", CURRENT_PHASE);
        printf("Available tests: Phase 9, 10, 11\n");
    }
    
    return 0;
} 