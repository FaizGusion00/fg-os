# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Development\fg-os

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Development\fg-os\build

# Utility rule file for phase3-validation.

# Include any custom commands dependencies for this target.
include boot/CMakeFiles/phase3-validation.dir/compiler_depend.make

# Include the progress variables for this target.
include boot/CMakeFiles/phase3-validation.dir/progress.make

boot/CMakeFiles/phase3-validation:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=C:\Development\fg-os\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Validating Phase 3 bootloader development environment"
	cd /d C:\Development\fg-os\build\boot && "C:\Program Files\CMake\bin\cmake.exe" -E echo "=== Phase 3 Validation ==="
	cd /d C:\Development\fg-os\build\boot && "C:\Program Files\CMake\bin\cmake.exe" -E echo "Bootloader: fg-boot v1.0.0"
	cd /d C:\Development\fg-os\build\boot && "C:\Program Files\CMake\bin\cmake.exe" -E echo "NASM: "
	cd /d C:\Development\fg-os\build\boot && "C:\Program Files\CMake\bin\cmake.exe" -E echo "GRUB: GRUB_MKRESCUE-NOTFOUND"
	cd /d C:\Development\fg-os\build\boot && "C:\Program Files\CMake\bin\cmake.exe" -E echo "QEMU: QEMU_X86_64-NOTFOUND"

boot/CMakeFiles/phase3-validation.dir/codegen:
.PHONY : boot/CMakeFiles/phase3-validation.dir/codegen

phase3-validation: boot/CMakeFiles/phase3-validation
phase3-validation: boot/CMakeFiles/phase3-validation.dir/build.make
.PHONY : phase3-validation

# Rule to build all files generated by this target.
boot/CMakeFiles/phase3-validation.dir/build: phase3-validation
.PHONY : boot/CMakeFiles/phase3-validation.dir/build

boot/CMakeFiles/phase3-validation.dir/clean:
	cd /d C:\Development\fg-os\build\boot && $(CMAKE_COMMAND) -P CMakeFiles\phase3-validation.dir\cmake_clean.cmake
.PHONY : boot/CMakeFiles/phase3-validation.dir/clean

boot/CMakeFiles/phase3-validation.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Development\fg-os C:\Development\fg-os\boot C:\Development\fg-os\build C:\Development\fg-os\build\boot C:\Development\fg-os\build\boot\CMakeFiles\phase3-validation.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : boot/CMakeFiles/phase3-validation.dir/depend

