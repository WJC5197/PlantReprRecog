# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

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
CMAKE_SOURCE_DIR = /root/code/cpp/PlantReprRecog

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/code/cpp/PlantReprRecog/build

# Include any dependencies generated for this target.
include CMakeFiles/thp.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/thp.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/thp.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/thp.dir/flags.make

CMakeFiles/thp.dir/src/threshold.cpp.o: CMakeFiles/thp.dir/flags.make
CMakeFiles/thp.dir/src/threshold.cpp.o: /root/code/cpp/PlantReprRecog/src/threshold.cpp
CMakeFiles/thp.dir/src/threshold.cpp.o: CMakeFiles/thp.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/code/cpp/PlantReprRecog/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/thp.dir/src/threshold.cpp.o"
	/usr/sbin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/thp.dir/src/threshold.cpp.o -MF CMakeFiles/thp.dir/src/threshold.cpp.o.d -o CMakeFiles/thp.dir/src/threshold.cpp.o -c /root/code/cpp/PlantReprRecog/src/threshold.cpp

CMakeFiles/thp.dir/src/threshold.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/thp.dir/src/threshold.cpp.i"
	/usr/sbin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/code/cpp/PlantReprRecog/src/threshold.cpp > CMakeFiles/thp.dir/src/threshold.cpp.i

CMakeFiles/thp.dir/src/threshold.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/thp.dir/src/threshold.cpp.s"
	/usr/sbin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/code/cpp/PlantReprRecog/src/threshold.cpp -o CMakeFiles/thp.dir/src/threshold.cpp.s

# Object files for target thp
thp_OBJECTS = \
"CMakeFiles/thp.dir/src/threshold.cpp.o"

# External object files for target thp
thp_EXTERNAL_OBJECTS =

thp: CMakeFiles/thp.dir/src/threshold.cpp.o
thp: CMakeFiles/thp.dir/build.make
thp: /usr/lib/libopencv_gapi.so.4.6.0
thp: /usr/lib/libopencv_stitching.so.4.6.0
thp: /usr/lib/libopencv_alphamat.so.4.6.0
thp: /usr/lib/libopencv_aruco.so.4.6.0
thp: /usr/lib/libopencv_barcode.so.4.6.0
thp: /usr/lib/libopencv_bgsegm.so.4.6.0
thp: /usr/lib/libopencv_bioinspired.so.4.6.0
thp: /usr/lib/libopencv_ccalib.so.4.6.0
thp: /usr/lib/libopencv_cvv.so.4.6.0
thp: /usr/lib/libopencv_dnn_objdetect.so.4.6.0
thp: /usr/lib/libopencv_dnn_superres.so.4.6.0
thp: /usr/lib/libopencv_dpm.so.4.6.0
thp: /usr/lib/libopencv_face.so.4.6.0
thp: /usr/lib/libopencv_freetype.so.4.6.0
thp: /usr/lib/libopencv_fuzzy.so.4.6.0
thp: /usr/lib/libopencv_hdf.so.4.6.0
thp: /usr/lib/libopencv_hfs.so.4.6.0
thp: /usr/lib/libopencv_img_hash.so.4.6.0
thp: /usr/lib/libopencv_intensity_transform.so.4.6.0
thp: /usr/lib/libopencv_line_descriptor.so.4.6.0
thp: /usr/lib/libopencv_mcc.so.4.6.0
thp: /usr/lib/libopencv_quality.so.4.6.0
thp: /usr/lib/libopencv_rapid.so.4.6.0
thp: /usr/lib/libopencv_reg.so.4.6.0
thp: /usr/lib/libopencv_rgbd.so.4.6.0
thp: /usr/lib/libopencv_saliency.so.4.6.0
thp: /usr/lib/libopencv_stereo.so.4.6.0
thp: /usr/lib/libopencv_structured_light.so.4.6.0
thp: /usr/lib/libopencv_superres.so.4.6.0
thp: /usr/lib/libopencv_surface_matching.so.4.6.0
thp: /usr/lib/libopencv_tracking.so.4.6.0
thp: /usr/lib/libopencv_videostab.so.4.6.0
thp: /usr/lib/libopencv_viz.so.4.6.0
thp: /usr/lib/libopencv_wechat_qrcode.so.4.6.0
thp: /usr/lib/libopencv_xfeatures2d.so.4.6.0
thp: /usr/lib/libopencv_xobjdetect.so.4.6.0
thp: /usr/lib/libopencv_xphoto.so.4.6.0
thp: /usr/lib/libopencv_shape.so.4.6.0
thp: /usr/lib/libopencv_highgui.so.4.6.0
thp: /usr/lib/libopencv_datasets.so.4.6.0
thp: /usr/lib/libopencv_plot.so.4.6.0
thp: /usr/lib/libopencv_text.so.4.6.0
thp: /usr/lib/libopencv_ml.so.4.6.0
thp: /usr/lib/libopencv_phase_unwrapping.so.4.6.0
thp: /usr/lib/libopencv_optflow.so.4.6.0
thp: /usr/lib/libopencv_ximgproc.so.4.6.0
thp: /usr/lib/libopencv_video.so.4.6.0
thp: /usr/lib/libopencv_videoio.so.4.6.0
thp: /usr/lib/libopencv_imgcodecs.so.4.6.0
thp: /usr/lib/libopencv_objdetect.so.4.6.0
thp: /usr/lib/libopencv_calib3d.so.4.6.0
thp: /usr/lib/libopencv_dnn.so.4.6.0
thp: /usr/lib/libopencv_features2d.so.4.6.0
thp: /usr/lib/libopencv_flann.so.4.6.0
thp: /usr/lib/libopencv_photo.so.4.6.0
thp: /usr/lib/libopencv_imgproc.so.4.6.0
thp: /usr/lib/libopencv_core.so.4.6.0
thp: CMakeFiles/thp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/code/cpp/PlantReprRecog/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable thp"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/thp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/thp.dir/build: thp
.PHONY : CMakeFiles/thp.dir/build

CMakeFiles/thp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/thp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/thp.dir/clean

CMakeFiles/thp.dir/depend:
	cd /root/code/cpp/PlantReprRecog/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/code/cpp/PlantReprRecog /root/code/cpp/PlantReprRecog /root/code/cpp/PlantReprRecog/build /root/code/cpp/PlantReprRecog/build /root/code/cpp/PlantReprRecog/build/CMakeFiles/thp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/thp.dir/depend

