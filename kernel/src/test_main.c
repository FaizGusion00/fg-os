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
    // Initialize basic console output
    printf("Starting FG-OS Phase 9 Hardware Abstraction Layer Test...\n\n");
    
    // Run Phase 9 HAL demonstration
    test_phase9();
    
    return 0;
} 