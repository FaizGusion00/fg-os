/**
 * @file test_framework.h
 * @brief FG-OS Test Framework Header
 * 
 * This file defines the test framework interface for FG-OS development.
 * It provides structures, macros, and function prototypes for testing.
 * 
 * @author Faiz Nasir
 * @company FGCompany Official
 * @version 1.0.0
 * @date 2024
 * @copyright © 2024 FGCompany Official. All rights reserved.
 */

#ifndef __TEST_FRAMEWORK_H__
#define __TEST_FRAMEWORK_H__

#include <stdint.h>
#include <stdbool.h>

// Test framework configuration
#define MAX_TESTS_PER_SUITE     100
#define MAX_TEST_NAME_LENGTH    128
#define MAX_TEST_DESC_LENGTH    256
#define MAX_FAILURE_MSG_LENGTH  512

// Test result codes
enum test_result {
    TEST_SUCCESS = 0,           /**< Test passed */
    TEST_FAILURE = 1,           /**< Test failed */
    TEST_ERROR = 2,             /**< Test error */
    TEST_SKIPPED = 3,           /**< Test skipped */
    TEST_TIMEOUT = 4            /**< Test timed out */
};

// Test status
enum test_status {
    TEST_STATUS_NOT_RUN = 0,    /**< Test not yet run */
    TEST_STATUS_RUNNING,        /**< Test currently running */
    TEST_STATUS_PASSED,         /**< Test passed */
    TEST_STATUS_FAILED,         /**< Test failed */
    TEST_STATUS_SKIPPED,        /**< Test skipped */
    TEST_STATUS_TIMEOUT         /**< Test timed out */
};

// Test function type
typedef enum test_result (*test_func_t)(void);

// Test case structure
struct test_case {
    char name[MAX_TEST_NAME_LENGTH];                /**< Test name */
    char description[MAX_TEST_DESC_LENGTH];         /**< Test description */
    test_func_t test_func;                          /**< Test function */
    enum test_status status;                        /**< Test status */
    uint64_t execution_time_ms;                     /**< Execution time in ms */
    char failure_message[MAX_FAILURE_MSG_LENGTH];   /**< Failure message */
};

// Test statistics
struct test_statistics {
    uint32_t total_tests;       /**< Total number of tests */
    uint32_t passed_tests;      /**< Number of passed tests */
    uint32_t failed_tests;      /**< Number of failed tests */
    uint32_t skipped_tests;     /**< Number of skipped tests */
    uint32_t timeout_tests;     /**< Number of timed out tests */
};

// Test suite structure
struct test_suite {
    char name[MAX_TEST_NAME_LENGTH];                /**< Suite name */
    char description[MAX_TEST_DESC_LENGTH];         /**< Suite description */
    struct test_case tests[MAX_TESTS_PER_SUITE];    /**< Test cases */
    uint32_t test_count;                            /**< Number of tests */
    test_func_t setup;                              /**< Setup function */
    test_func_t teardown;                           /**< Teardown function */
    struct test_statistics stats;                   /**< Suite statistics */
};

// Test configuration
struct test_config {
    bool verbose;               /**< Verbose output */
    bool color_output;          /**< Color output */
    bool stop_on_failure;       /**< Stop on first failure */
    uint32_t timeout_seconds;   /**< Test timeout in seconds */
};

// Framework initialization and cleanup
int  test_framework_init(void);
void test_framework_cleanup(void);

// Test suite management
struct test_suite *test_suite_create(const char *name, const char *description);
void test_suite_destroy(struct test_suite *suite);
int  test_suite_add_test(struct test_suite *suite, const char *name, 
                        test_func_t test_func, const char *description);
int  test_suite_run(struct test_suite *suite);

// Test utilities
void test_set_failure_message(const char *format, ...);
void test_print_statistics(void);
struct test_statistics test_get_statistics(void);

// Configuration
void test_set_config(struct test_config *config);
struct test_config test_get_config(void);

// Output functions
void print_test_banner(void);

// Test assertion macros
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            test_set_failure_message("Assertion failed: %s at %s:%d", \
                                    #condition, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            test_set_failure_message("Expected %ld, got %ld at %s:%d", \
                                    (long)(expected), (long)(actual), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            test_set_failure_message("Expected not %ld, got %ld at %s:%d", \
                                    (long)(expected), (long)(actual), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            test_set_failure_message("Expected NULL, got %p at %s:%d", \
                                    (ptr), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            test_set_failure_message("Expected non-NULL pointer at %s:%d", \
                                    __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            test_set_failure_message("Expected '%s', got '%s' at %s:%d", \
                                    (expected), (actual), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_MEM_EQ(expected, actual, size) \
    do { \
        if (memcmp((expected), (actual), (size)) != 0) { \
            test_set_failure_message("Memory comparison failed at %s:%d", \
                                    __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_FAIL(message) \
    do { \
        test_set_failure_message("%s at %s:%d", (message), __FILE__, __LINE__); \
        return TEST_FAILURE; \
    } while (0)

#define TEST_SKIP(message) \
    do { \
        test_set_failure_message("SKIPPED: %s at %s:%d", (message), __FILE__, __LINE__); \
        return TEST_SKIPPED; \
    } while (0)

// Test suite registration macros
#define TEST_SUITE_DECLARE(suite_name) \
    extern struct test_suite suite_name

#define TEST_SUITE_DEFINE(suite_name, description) \
    struct test_suite suite_name = { \
        .name = #suite_name, \
        .description = description, \
        .test_count = 0, \
        .setup = NULL, \
        .teardown = NULL, \
        .stats = {0} \
    }

#define TEST_CASE(test_name) \
    static enum test_result test_##test_name(void)

#define TEST_REGISTER(suite, test_name, description) \
    test_suite_add_test(&(suite), #test_name, test_##test_name, description)

// Phase 2 specific test macros
#define TEST_PHASE2_ASSERT(condition, phase_desc) \
    do { \
        if (!(condition)) { \
            test_set_failure_message("Phase 2 requirement failed: %s - %s at %s:%d", \
                                    phase_desc, #condition, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

// Development environment test macros
#define TEST_ASSERT_BUILD_SUCCESS(result) \
    TEST_ASSERT_EQ(0, result)

#define TEST_ASSERT_COMPILER_AVAILABLE(compiler) \
    do { \
        if (system("which " compiler " > /dev/null 2>&1") != 0) { \
            test_set_failure_message("Compiler '%s' not available at %s:%d", \
                                    compiler, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_TOOL_AVAILABLE(tool) \
    do { \
        if (system("which " tool " > /dev/null 2>&1") != 0) { \
            test_set_failure_message("Tool '%s' not available at %s:%d", \
                                    tool, __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

// Test timing macros
#define TEST_START_TIMER() \
    uint64_t _test_start_time = 0  // TODO: Implement proper timing

#define TEST_END_TIMER() \
    uint64_t _test_end_time = 0    // TODO: Implement proper timing

#define TEST_ASSERT_TIME_UNDER(max_ms) \
    do { \
        uint64_t elapsed = _test_end_time - _test_start_time; \
        if (elapsed > (max_ms)) { \
            test_set_failure_message("Test took %lu ms, expected under %lu ms at %s:%d", \
                                    elapsed, (uint64_t)(max_ms), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

// FG-OS specific test macros
#define TEST_ASSERT_FGOS_VERSION(expected_version) \
    TEST_ASSERT_STR_EQ(expected_version, KERNEL_VERSION)

#define TEST_ASSERT_FGOS_PHASE(expected_phase) \
    TEST_ASSERT_EQ(expected_phase, CURRENT_PHASE)

#define TEST_ASSERT_KERNEL_INITIALIZED() \
    TEST_ASSERT(kernel_initialized)

// Memory test macros
#define TEST_ASSERT_MEMORY_ALIGNED(ptr, alignment) \
    do { \
        if (((uintptr_t)(ptr)) % (alignment) != 0) { \
            test_set_failure_message("Memory not aligned to %u bytes at %s:%d", \
                                    (alignment), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define TEST_ASSERT_VALID_KERNEL_ADDRESS(addr) \
    do { \
        if ((uint64_t)(addr) < KERNEL_VIRTUAL_BASE) { \
            test_set_failure_message("Invalid kernel address 0x%lx at %s:%d", \
                                    (uint64_t)(addr), __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

// Test runner main function
#define TEST_MAIN() \
    int main(int argc, char *argv[]) { \
        (void)argc; (void)argv; \
        \
        test_framework_init(); \
        \
        /* Run all test suites */ \
        run_all_tests(); \
        \
        /* Print final statistics */ \
        test_print_statistics(); \
        \
        /* Cleanup */ \
        test_framework_cleanup(); \
        \
        /* Return exit code based on results */ \
        struct test_statistics stats = test_get_statistics(); \
        return (stats.failed_tests == 0) ? 0 : 1; \
    }

// Function prototype for test runner
void run_all_tests(void);

#endif /* __TEST_FRAMEWORK_H__ */ 