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
CMAKE_SOURCE_DIR = /home/saanders/Desktop/MC937/MC937-animation

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/saanders/Desktop/MC937/MC937-animation

# Include any dependencies generated for this target.
include CMakeFiles/scene2.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/scene2.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/scene2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/scene2.dir/flags.make

CMakeFiles/scene2.dir/scene2.cpp.o: CMakeFiles/scene2.dir/flags.make
CMakeFiles/scene2.dir/scene2.cpp.o: scene2.cpp
CMakeFiles/scene2.dir/scene2.cpp.o: CMakeFiles/scene2.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/saanders/Desktop/MC937/MC937-animation/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/scene2.dir/scene2.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/scene2.dir/scene2.cpp.o -MF CMakeFiles/scene2.dir/scene2.cpp.o.d -o CMakeFiles/scene2.dir/scene2.cpp.o -c /home/saanders/Desktop/MC937/MC937-animation/scene2.cpp

CMakeFiles/scene2.dir/scene2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/scene2.dir/scene2.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/saanders/Desktop/MC937/MC937-animation/scene2.cpp > CMakeFiles/scene2.dir/scene2.cpp.i

CMakeFiles/scene2.dir/scene2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/scene2.dir/scene2.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/saanders/Desktop/MC937/MC937-animation/scene2.cpp -o CMakeFiles/scene2.dir/scene2.cpp.s

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
scene2: /usr/lib/x86_64-linux-gnu/libopencv_stitching.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_alphamat.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_aruco.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_barcode.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_bgsegm.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_bioinspired.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_ccalib.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_dnn_objdetect.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_dnn_superres.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_dpm.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_face.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_freetype.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_fuzzy.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_hdf.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_hfs.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_img_hash.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_intensity_transform.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_line_descriptor.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_mcc.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_quality.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_rapid.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_reg.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_rgbd.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_saliency.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_shape.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_stereo.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_structured_light.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_superres.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_surface_matching.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_tracking.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_videostab.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_viz.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_wechat_qrcode.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_xobjdetect.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_xphoto.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libglfw.so.3.3
scene2: /usr/lib/x86_64-linux-gnu/libGLEW.so
scene2: /usr/lib/x86_64-linux-gnu/libopencv_highgui.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_datasets.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_plot.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_text.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_ml.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_phase_unwrapping.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_optflow.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_ximgproc.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_video.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_videoio.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_imgcodecs.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_objdetect.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_calib3d.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_dnn.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_features2d.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_flann.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_photo.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_imgproc.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libopencv_core.so.4.5.4d
scene2: /usr/lib/x86_64-linux-gnu/libGLX.so
scene2: /usr/lib/x86_64-linux-gnu/libOpenGL.so
scene2: CMakeFiles/scene2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/saanders/Desktop/MC937/MC937-animation/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable scene2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/scene2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/scene2.dir/build: scene2
.PHONY : CMakeFiles/scene2.dir/build

CMakeFiles/scene2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/scene2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/scene2.dir/clean

CMakeFiles/scene2.dir/depend:
	cd /home/saanders/Desktop/MC937/MC937-animation && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/saanders/Desktop/MC937/MC937-animation /home/saanders/Desktop/MC937/MC937-animation /home/saanders/Desktop/MC937/MC937-animation /home/saanders/Desktop/MC937/MC937-animation /home/saanders/Desktop/MC937/MC937-animation/CMakeFiles/scene2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/scene2.dir/depend

