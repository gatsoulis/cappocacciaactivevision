# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.6

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build

# Include any dependencies generated for this target.
include CMakeFiles/findLocationSURF.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/findLocationSURF.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/findLocationSURF.dir/flags.make

CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o: CMakeFiles/findLocationSURF.dir/flags.make
CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o: ../findLocationSURF.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o -c /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/findLocationSURF.cpp

CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/findLocationSURF.cpp > CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.i

CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/findLocationSURF.cpp -o CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.s

CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.requires:
.PHONY : CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.requires

CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.provides: CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.requires
	$(MAKE) -f CMakeFiles/findLocationSURF.dir/build.make CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.provides.build
.PHONY : CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.provides

CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.provides.build: CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o
.PHONY : CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.provides.build

CMakeFiles/findLocationSURF.dir/SURF.cpp.o: CMakeFiles/findLocationSURF.dir/flags.make
CMakeFiles/findLocationSURF.dir/SURF.cpp.o: ../SURF.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/findLocationSURF.dir/SURF.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/findLocationSURF.dir/SURF.cpp.o -c /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/SURF.cpp

CMakeFiles/findLocationSURF.dir/SURF.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/findLocationSURF.dir/SURF.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/SURF.cpp > CMakeFiles/findLocationSURF.dir/SURF.cpp.i

CMakeFiles/findLocationSURF.dir/SURF.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/findLocationSURF.dir/SURF.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/SURF.cpp -o CMakeFiles/findLocationSURF.dir/SURF.cpp.s

CMakeFiles/findLocationSURF.dir/SURF.cpp.o.requires:
.PHONY : CMakeFiles/findLocationSURF.dir/SURF.cpp.o.requires

CMakeFiles/findLocationSURF.dir/SURF.cpp.o.provides: CMakeFiles/findLocationSURF.dir/SURF.cpp.o.requires
	$(MAKE) -f CMakeFiles/findLocationSURF.dir/build.make CMakeFiles/findLocationSURF.dir/SURF.cpp.o.provides.build
.PHONY : CMakeFiles/findLocationSURF.dir/SURF.cpp.o.provides

CMakeFiles/findLocationSURF.dir/SURF.cpp.o.provides.build: CMakeFiles/findLocationSURF.dir/SURF.cpp.o
.PHONY : CMakeFiles/findLocationSURF.dir/SURF.cpp.o.provides.build

# Object files for target findLocationSURF
findLocationSURF_OBJECTS = \
"CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o" \
"CMakeFiles/findLocationSURF.dir/SURF.cpp.o"

# External object files for target findLocationSURF
findLocationSURF_EXTERNAL_OBJECTS =

findLocationSURF: CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o
findLocationSURF: CMakeFiles/findLocationSURF.dir/SURF.cpp.o
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_OS.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_sig.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_math.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_dev.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libyarpmod.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libyarp_serial.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_init.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libyarpmod.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libyarp_serial.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_math.a
findLocationSURF: /usr/lib/libgsl.so
findLocationSURF: /usr/lib/libgslcblas.so
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_dev.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_sig.a
findLocationSURF: /usr/local/src/robot/yarp2/build/lib/libYARP_OS.a
findLocationSURF: CMakeFiles/findLocationSURF.dir/build.make
findLocationSURF: CMakeFiles/findLocationSURF.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable findLocationSURF"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/findLocationSURF.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/findLocationSURF.dir/build: findLocationSURF
.PHONY : CMakeFiles/findLocationSURF.dir/build

CMakeFiles/findLocationSURF.dir/requires: CMakeFiles/findLocationSURF.dir/findLocationSURF.cpp.o.requires
CMakeFiles/findLocationSURF.dir/requires: CMakeFiles/findLocationSURF.dir/SURF.cpp.o.requires
.PHONY : CMakeFiles/findLocationSURF.dir/requires

CMakeFiles/findLocationSURF.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/findLocationSURF.dir/cmake_clean.cmake
.PHONY : CMakeFiles/findLocationSURF.dir/clean

CMakeFiles/findLocationSURF.dir/depend:
	cd /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build /usr/local/src/robot/Contract_meeting/8_ObjectTrackingSURF/build/CMakeFiles/findLocationSURF.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/findLocationSURF.dir/depend

