# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/thigs/Documentos/MC937-animation

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/thigs/Documentos/MC937-animation/build

# Include any dependencies generated for this target.
include CMakeFiles/scene2.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/scene2.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/scene2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/scene2.dir/flags.make

CMakeFiles/scene2.dir/scene2.cpp.o: CMakeFiles/scene2.dir/flags.make
CMakeFiles/scene2.dir/scene2.cpp.o: ../scene2.cpp
CMakeFiles/scene2.dir/scene2.cpp.o: CMakeFiles/scene2.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/thigs/Documentos/MC937-animation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/scene2.dir/scene2.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/scene2.dir/scene2.cpp.o -MF CMakeFiles/scene2.dir/scene2.cpp.o.d -o CMakeFiles/scene2.dir/scene2.cpp.o -c /home/thigs/Documentos/MC937-animation/scene2.cpp

CMakeFiles/scene2.dir/scene2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/scene2.dir/scene2.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/thigs/Documentos/MC937-animation/scene2.cpp > CMakeFiles/scene2.dir/scene2.cpp.i

CMakeFiles/scene2.dir/scene2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/scene2.dir/scene2.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/thigs/Documentos/MC937-animation/scene2.cpp -o CMakeFiles/scene2.dir/scene2.cpp.s

# Object files for target scene2
scene2_OBJECTS = \
"CMakeFiles/scene2.dir/scene2.cpp.o"

# External object files for target scene2
scene2_EXTERNAL_OBJECTS =

scene2: CMakeFiles/scene2.dir/scene2.cpp.o
scene2: CMakeFiles/scene2.dir/build.make
scene2: libcollision.a
scene2: libphysics.a
scene2: libraycast.a
scene2: libloader.a
scene2: /usr/local/lib/libopencv_gapi.so.4.12.0
scene2: /usr/local/lib/libopencv_highgui.so.4.12.0
scene2: /usr/local/lib/libopencv_ml.so.4.12.0
scene2: /usr/local/lib/libopencv_objdetect.so.4.12.0
scene2: /usr/local/lib/libopencv_photo.so.4.12.0
scene2: /usr/local/lib/libopencv_stitching.so.4.12.0
scene2: /usr/local/lib/libopencv_video.so.4.12.0
scene2: /usr/local/lib/libopencv_videoio.so.4.12.0
scene2: /usr/lib/x86_64-linux-gnu/libglfw.so.3.3
scene2: /usr/lib/x86_64-linux-gnu/libGLEW.so
scene2: /usr/local/lib/libopencv_imgcodecs.so.4.12.0
scene2: /usr/local/lib/libopencv_dnn.so.4.12.0
scene2: /usr/local/lib/libopencv_calib3d.so.4.12.0
scene2: /usr/local/lib/libopencv_features2d.so.4.12.0
scene2: /usr/local/lib/libopencv_flann.so.4.12.0
scene2: /usr/local/lib/libopencv_imgproc.so.4.12.0
scene2: /usr/local/lib/libopencv_core.so.4.12.0
scene2: /usr/lib/x86_64-linux-gnu/libGLX.so
scene2: /usr/lib/x86_64-linux-gnu/libOpenGL.so
scene2: CMakeFiles/scene2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/thigs/Documentos/MC937-animation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable scene2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/scene2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/scene2.dir/build: scene2
.PHONY : CMakeFiles/scene2.dir/build

CMakeFiles/scene2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/scene2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/scene2.dir/clean

CMakeFiles/scene2.dir/depend:
	cd /home/thigs/Documentos/MC937-animation/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/thigs/Documentos/MC937-animation /home/thigs/Documentos/MC937-animation /home/thigs/Documentos/MC937-animation/build /home/thigs/Documentos/MC937-animation/build /home/thigs/Documentos/MC937-animation/build/CMakeFiles/scene2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/scene2.dir/depend

