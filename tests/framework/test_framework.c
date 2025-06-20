/**
 * @file test_framework.c
 * @brief FG-OS Test Framework Implementation
 * 
 * This file implements the core testing framework for FG-OS development.
 * It provides test registration, execution, and reporting capabilities.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "test_framework.h"

// Global test state
static struct test_suite *current_suite = NULL;
static struct test_case *current_test = NULL;
static struct test_statistics global_stats = {0};
static bool framework_initialized = false;

// Test output configuration
static struct test_config config = {
    .verbose = true,
    .color_output = true,
    .stop_on_failure = false,
    .timeout_seconds = 30
};

/**
 * @brief Initialize the test framework
 */
int test_framework_init(void) {
    if (framework_initialized) {
        return TEST_SUCCESS;
    }
    
    // Reset global statistics
    memset(&global_stats, 0, sizeof(global_stats));
    
    // Initialize framework
    framework_initialized = true;
    
    // Print framework banner
    if (config.verbose) {
        print_test_banner();
    }
    
    return TEST_SUCCESS;
}

/**
 * @brief Cleanup the test framework
 */
void test_framework_cleanup(void) {
    framework_initialized = false;
    current_suite = NULL;
    current_test = NULL;
}

/**
 * @brief Print the test framework banner
 */
void print_test_banner(void) {
    if (config.color_output) {
        printf("\033[96m"); // Cyan
    }
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║               FG-OS Testing Framework v1.0.0                ║\n");
    printf("║                                                              ║\n");
    printf("║  Phase 2: Development Environment Setup Tests               ║\n");
    printf("║  Developed by: Faiz Nasir - FGCompany Official             ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (config.color_output) {
        printf("\033[0m"); // Reset
    }
    printf("\n");
}

/**
 * @brief Create a new test suite
 */
struct test_suite *test_suite_create(const char *name, const char *description) {
    if (!name) {
        return NULL;
    }
    
    struct test_suite *suite = malloc(sizeof(struct test_suite));
    if (!suite) {
        return NULL;
    }
    
    // Initialize suite
    strncpy(suite->name, name, sizeof(suite->name) - 1);
    suite->name[sizeof(suite->name) - 1] = '\0';
    
    if (description) {
        strncpy(suite->description, description, sizeof(suite->description) - 1);
        suite->description[sizeof(suite->description) - 1] = '\0';
    } else {
        suite->description[0] = '\0';
    }
    
    suite->test_count = 0;
    suite->setup = NULL;
    suite->teardown = NULL;
    memset(&suite->stats, 0, sizeof(suite->stats));
    
    return suite;
}

/**
 * @brief Add a test case to a suite
 */
int test_suite_add_test(struct test_suite *suite, const char *name, 
                       test_func_t test_func, const char *description) {
    if (!suite || !name || !test_func || suite->test_count >= MAX_TESTS_PER_SUITE) {
        return TEST_ERROR;
    }
    
    struct test_case *test = &suite->tests[suite->test_count];
    
    // Initialize test case
    strncpy(test->name, name, sizeof(test->name) - 1);
    test->name[sizeof(test->name) - 1] = '\0';
    
    if (description) {
        strncpy(test->description, description, sizeof(test->description) - 1);
        test->description[sizeof(test->description) - 1] = '\0';
    } else {
        test->description[0] = '\0';
    }
    
    test->test_func = test_func;
    test->status = TEST_STATUS_NOT_RUN;
    test->execution_time_ms = 0;
    test->failure_message[0] = '\0';
    
    suite->test_count++;
    return TEST_SUCCESS;
}

/**
 * @brief Run a single test case
 */
static enum test_result run_test_case(struct test_suite *suite, struct test_case *test) {
    if (!suite || !test || !test->test_func) {
        return TEST_ERROR;
    }
    
    current_test = test;
    test->status = TEST_STATUS_RUNNING;
    
    // Print test start message
    if (config.verbose) {
        printf("  Running: %s", test->name);
        if (test->description[0]) {
            printf(" - %s", test->description);
        }
        printf("...");
        fflush(stdout);
    }
    
    // Record start time (simple implementation)
    // TODO: Implement proper timing in later phases
    uint64_t start_time = 0;
    
    // Run test function
    enum test_result result = test->test_func();
    
    // Record end time and calculate duration
    uint64_t end_time = 0;
    test->execution_time_ms = end_time - start_time;
    
    // Update test status
    test->status = (result == TEST_SUCCESS) ? TEST_STATUS_PASSED : TEST_STATUS_FAILED;
    
    // Print result
    if (config.verbose) {
        if (result == TEST_SUCCESS) {
            if (config.color_output) {
                printf(" \033[92mPASS\033[0m\n"); // Green
            } else {
                printf(" PASS\n");
            }
        } else {
            if (config.color_output) {
                printf(" \033[91mFAIL\033[0m\n"); // Red
            } else {
                printf(" FAIL\n");
            }
            
            if (test->failure_message[0]) {
                printf("    Reason: %s\n", test->failure_message);
            }
        }
    }
    
    current_test = NULL;
    return result;
}

/**
 * @brief Run all tests in a suite
 */
int test_suite_run(struct test_suite *suite) {
    if (!suite) {
        return TEST_ERROR;
    }
    
    current_suite = suite;
    
    // Print suite header
    if (config.verbose) {
        printf("\n");
        if (config.color_output) {
            printf("\033[94m"); // Blue
        }
        printf("═══ Test Suite: %s ═══\n", suite->name);
        if (suite->description[0]) {
            printf("Description: %s\n", suite->description);
        }
        if (config.color_output) {
            printf("\033[0m"); // Reset
        }
        printf("\n");
    }
    
    // Run suite setup if available
    if (suite->setup) {
        if (suite->setup() != TEST_SUCCESS) {
            printf("Suite setup failed!\n");
            return TEST_ERROR;
        }
    }
    
    // Run all tests
    for (int i = 0; i < suite->test_count; i++) {
        enum test_result result = run_test_case(suite, &suite->tests[i]);
        
        // Update statistics
        suite->stats.total_tests++;
        global_stats.total_tests++;
        
        if (result == TEST_SUCCESS) {
            suite->stats.passed_tests++;
            global_stats.passed_tests++;
        } else {
            suite->stats.failed_tests++;
            global_stats.failed_tests++;
            
            if (config.stop_on_failure) {
                break;
            }
        }
    }
    
    // Run suite teardown if available
    if (suite->teardown) {
        suite->teardown();
    }
    
    // Print suite summary
    if (config.verbose) {
        printf("\n");
        printf("Suite Results: %d passed, %d failed, %d total\n",
               suite->stats.passed_tests, suite->stats.failed_tests, 
               suite->stats.total_tests);
    }
    
    current_suite = NULL;
    return (suite->stats.failed_tests == 0) ? TEST_SUCCESS : TEST_FAILURE;
}

/**
 * @brief Set test failure message
 */
void test_set_failure_message(const char *format, ...) {
    if (!current_test) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vsnprintf(current_test->failure_message, sizeof(current_test->failure_message), 
              format, args);
    va_end(args);
}

/**
 * @brief Print global test statistics
 */
void test_print_statistics(void) {
    printf("\n");
    if (config.color_output) {
        printf("\033[95m"); // Magenta
    }
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                     Test Results Summary                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:   %4d                                       ║\n", global_stats.total_tests);
    printf("║  Passed Tests:  %4d                                       ║\n", global_stats.passed_tests);
    printf("║  Failed Tests:  %4d                                       ║\n", global_stats.failed_tests);
    printf("║                                                              ║\n");
    
    if (global_stats.total_tests > 0) {
        double pass_rate = (double)global_stats.passed_tests / global_stats.total_tests * 100.0;
        printf("║  Pass Rate:     %5.1f%%                                    ║\n", pass_rate);
    } else {
        printf("║  Pass Rate:     N/A                                        ║\n");
    }
    
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (config.color_output) {
        printf("\033[0m"); // Reset
    }
    printf("\n");
    
    // Overall result
    if (global_stats.failed_tests == 0 && global_stats.total_tests > 0) {
        if (config.color_output) {
            printf("\033[92m"); // Green
        }
        printf("✓ ALL TESTS PASSED!\n");
        if (config.color_output) {
            printf("\033[0m");
        }
    } else if (global_stats.failed_tests > 0) {
        if (config.color_output) {
            printf("\033[91m"); // Red
        }
        printf("✗ SOME TESTS FAILED!\n");
        if (config.color_output) {
            printf("\033[0m");
        }
    } else {
        printf("! NO TESTS RUN\n");
    }
}

/**
 * @brief Get global test statistics
 */
struct test_statistics test_get_statistics(void) {
    return global_stats;
}

/**
 * @brief Set test configuration
 */
void test_set_config(struct test_config *new_config) {
    if (new_config) {
        config = *new_config;
    }
}

/**
 * @brief Get current test configuration
 */
struct test_config test_get_config(void) {
    return config;
}

/**
 * @brief Clean up and free a test suite
 */
void test_suite_destroy(struct test_suite *suite) {
    if (suite) {
        free(suite);
    }
} 