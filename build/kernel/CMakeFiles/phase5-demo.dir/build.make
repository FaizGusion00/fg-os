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

# Include any dependencies generated for this target.
include kernel/CMakeFiles/phase5-demo.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include kernel/CMakeFiles/phase5-demo.dir/compiler_depend.make

# Include the progress variables for this target.
include kernel/CMakeFiles/phase5-demo.dir/progress.make

# Include the compile flags for this target's objects.
include kernel/CMakeFiles/phase5-demo.dir/flags.make

kernel/CMakeFiles/phase5-demo.dir/codegen:
.PHONY : kernel/CMakeFiles/phase5-demo.dir/codegen

kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj: kernel/CMakeFiles/phase5-demo.dir/flags.make
kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj: kernel/CMakeFiles/phase5-demo.dir/includes_C.rsp
kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj: C:/Development/fg-os/kernel/src/test_main.c
kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj: kernel/CMakeFiles/phase5-demo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Development\fg-os\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj"
	cd /d C:\Development\fg-os\build\kernel && C:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj -MF CMakeFiles\phase5-demo.dir\src\test_main.c.obj.d -o CMakeFiles\phase5-demo.dir\src\test_main.c.obj -c C:\Development\fg-os\kernel\src\test_main.c

kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/phase5-demo.dir/src/test_main.c.i"
	cd /d C:\Development\fg-os\build\kernel && C:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Development\fg-os\kernel\src\test_main.c > CMakeFiles\phase5-demo.dir\src\test_main.c.i

kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/phase5-demo.dir/src/test_main.c.s"
	cd /d C:\Development\fg-os\build\kernel && C:\msys64\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Development\fg-os\kernel\src\test_main.c -o CMakeFiles\phase5-demo.dir\src\test_main.c.s

# Object files for target phase5-demo
phase5__demo_OBJECTS = \
"CMakeFiles/phase5-demo.dir/src/test_main.c.obj"

# External object files for target phase5-demo
phase5__demo_EXTERNAL_OBJECTS =

kernel/phase5-demo.exe: kernel/CMakeFiles/phase5-demo.dir/src/test_main.c.obj
kernel/phase5-demo.exe: kernel/CMakeFiles/phase5-demo.dir/build.make
kernel/phase5-demo.exe: kernel/CMakeFiles/phase5-demo.dir/linkLibs.rsp
kernel/phase5-demo.exe: kernel/CMakeFiles/phase5-demo.dir/objects1.rsp
kernel/phase5-demo.exe: kernel/CMakeFiles/phase5-demo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Development\fg-os\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable phase5-demo.exe"
	cd /d C:\Development\fg-os\build\kernel && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\phase5-demo.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
kernel/CMakeFiles/phase5-demo.dir/build: kernel/phase5-demo.exe
.PHONY : kernel/CMakeFiles/phase5-demo.dir/build

kernel/CMakeFiles/phase5-demo.dir/clean:
	cd /d C:\Development\fg-os\build\kernel && $(CMAKE_COMMAND) -P CMakeFiles\phase5-demo.dir\cmake_clean.cmake
.PHONY : kernel/CMakeFiles/phase5-demo.dir/clean

kernel/CMakeFiles/phase5-demo.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Development\fg-os C:\Development\fg-os\kernel C:\Development\fg-os\build C:\Development\fg-os\build\kernel C:\Development\fg-os\build\kernel\CMakeFiles\phase5-demo.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : kernel/CMakeFiles/phase5-demo.dir/depend

