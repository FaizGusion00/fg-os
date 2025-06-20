# FG-OS Testing Framework
# Developed by: Faiz Nasir - FGCompany Official

cmake_minimum_required(VERSION 3.20)

# Test configuration
set(TEST_NAME "fg-os-tests")
set(TEST_VERSION "1.0.0")

# Include test source directory
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/include
)

# Test framework source files
set(TEST_FRAMEWORK_SOURCES
    framework/test_framework.c
    framework/test_runner.c
    framework/test_assertions.c
    framework/test_reporter.c
)

# Unit test source files
set(UNIT_TEST_SOURCES
    unit/test_types.c
    unit/test_kernel_init.c
    unit/test_panic.c
    unit/test_memory_layout.c
)

# Integration test source files
set(INTEGRATION_TEST_SOURCES
    integration/test_boot_sequence.c
    integration/test_kernel_subsystems.c
)

# System test source files (Phase 2: Basic tests only)
set(SYSTEM_TEST_SOURCES
    system/test_basic_functionality.c
    system/test_phase2_requirements.c
)

# Phase 2 specific tests
set(PHASE2_TEST_SOURCES
    phase2/test_build_system.c
    phase2/test_development_environment.c
    phase2/test_toolchain.c
)

# Create test framework library
add_library(test_framework STATIC ${TEST_FRAMEWORK_SOURCES})

# Create unit test executable
add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests test_framework)

# Create integration test executable
add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(integration_tests test_framework)

# Create system test executable
add_executable(system_tests ${SYSTEM_TEST_SOURCES})
target_link_libraries(system_tests test_framework)

# Create Phase 2 specific test executable
add_executable(phase2_tests ${PHASE2_TEST_SOURCES})
target_link_libraries(phase2_tests test_framework)

# Test targets
add_custom_target(test_all
    COMMAND unit_tests
    COMMAND integration_tests
    COMMAND system_tests
    COMMAND phase2_tests
    DEPENDS unit_tests integration_tests system_tests phase2_tests
    COMMENT "Running all FG-OS tests"
)

add_custom_target(test_unit
    COMMAND unit_tests
    DEPENDS unit_tests
    COMMENT "Running unit tests"
)

add_custom_target(test_integration
    COMMAND integration_tests
    DEPENDS integration_tests
    COMMENT "Running integration tests"
)

add_custom_target(test_system
    COMMAND system_tests
    DEPENDS system_tests
    COMMENT "Running system tests"
)

add_custom_target(test_phase2
    COMMAND phase2_tests
    DEPENDS phase2_tests
    COMMENT "Running Phase 2 specific tests"
)

# Test with coverage (if available)
find_program(GCOV_PATH gcov)
if(GCOV_PATH)
    add_custom_target(test_coverage
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_coverage.sh
        DEPENDS test_all
        COMMENT "Generating test coverage report"
    )
endif()

# CTest integration
enable_testing()

# Add individual test cases
add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME integration_tests COMMAND integration_tests)
add_test(NAME system_tests COMMAND system_tests)
add_test(NAME phase2_tests COMMAND phase2_tests)

# Test properties
set_tests_properties(unit_tests PROPERTIES
    TIMEOUT 30
    LABELS "unit;fast"
)

set_tests_properties(integration_tests PROPERTIES
    TIMEOUT 60
    LABELS "integration;medium"
)

set_tests_properties(system_tests PROPERTIES
    TIMEOUT 120
    LABELS "system;slow"
)

set_tests_properties(phase2_tests PROPERTIES
    TIMEOUT 60
    LABELS "phase2;development"
)

# Phase-specific test configuration
if(CURRENT_PHASE EQUAL 2)
    message(STATUS "Configuring Phase 2 testing framework...")
    
    # Add Phase 2 specific test definitions
    add_definitions(-DPHASE_2_TESTING)
    add_definitions(-DTEST_DEVELOPMENT_ENVIRONMENT)
    add_definitions(-DTEST_BUILD_SYSTEM)
    
    # Phase 2 test summary
    add_custom_target(phase2_test_summary
        COMMAND ${CMAKE_COMMAND} -E echo "Phase 2 Testing Summary:"
        COMMAND ${CMAKE_COMMAND} -E echo "- Build system tests: ENABLED"
        COMMAND ${CMAKE_COMMAND} -E echo "- Environment tests: ENABLED" 
        COMMAND ${CMAKE_COMMAND} -E echo "- Toolchain tests: ENABLED"
        COMMAND ${CMAKE_COMMAND} -E echo "- Unit tests: BASIC"
        COMMAND ${CMAKE_COMMAND} -E echo "- Integration tests: MINIMAL"
        COMMENT "Phase 2 testing configuration summary"
    )
endif()

# Test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# Test output directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_output)

# Install test executables
install(TARGETS unit_tests integration_tests system_tests phase2_tests
        DESTINATION bin/tests)

# Install test framework library
install(TARGETS test_framework
        DESTINATION lib)

# Print test configuration
message(STATUS "=== Test Configuration ===")
message(STATUS "Test Framework: ${TEST_NAME} v${TEST_VERSION}")
message(STATUS "Unit Tests: ${UNIT_TEST_SOURCES}")
message(STATUS "Integration Tests: ${INTEGRATION_TEST_SOURCES}")
message(STATUS "System Tests: ${SYSTEM_TEST_SOURCES}")
message(STATUS "Phase 2 Tests: ${PHASE2_TEST_SOURCES}")
message(STATUS "Coverage Tool: ${GCOV_PATH}")
message(STATUS "=============================") 