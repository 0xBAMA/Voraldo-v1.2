# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
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

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jb/Documents/Voraldo-v1.2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jb/Documents/Voraldo-v1.2

# Include any dependencies generated for this target.
include CMakeFiles/imgui.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/imgui.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/imgui.dir/flags.make

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o: resources/ocornut_imgui/imgui_impl_sdl.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_impl_sdl.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_impl_sdl.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_impl_sdl.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o: resources/ocornut_imgui/imgui_impl_opengl3.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_impl_opengl3.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_impl_opengl3.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_impl_opengl3.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o: resources/ocornut_imgui/imgui.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o: resources/ocornut_imgui/imgui_demo.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_demo.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_demo.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_demo.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o: resources/ocornut_imgui/imgui_draw.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_draw.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_draw.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_draw.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o: resources/ocornut_imgui/imgui_widgets.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_widgets.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_widgets.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_widgets.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o: resources/ocornut_imgui/imgui_tables.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_tables.cc

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_tables.cc > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_tables.cc -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o


CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o: CMakeFiles/imgui.dir/flags.make
CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o: resources/ocornut_imgui/imgui_gl3w.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o   -c /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_gl3w.c

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_gl3w.c > CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.i

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jb/Documents/Voraldo-v1.2/resources/ocornut_imgui/imgui_gl3w.c -o CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.s

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.requires:

.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.requires

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.provides: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.requires
	$(MAKE) -f CMakeFiles/imgui.dir/build.make CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.provides.build
.PHONY : CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.provides

CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.provides.build: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o


# Object files for target imgui
imgui_OBJECTS = \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o" \
"CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o"

# External object files for target imgui
imgui_EXTERNAL_OBJECTS =

libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o
libimgui.a: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o
libimgui.a: CMakeFiles/imgui.dir/build.make
libimgui.a: CMakeFiles/imgui.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jb/Documents/Voraldo-v1.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX static library libimgui.a"
	$(CMAKE_COMMAND) -P CMakeFiles/imgui.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imgui.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/imgui.dir/build: libimgui.a

.PHONY : CMakeFiles/imgui.dir/build

CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_sdl.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_impl_opengl3.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_demo.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_draw.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_widgets.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_tables.cc.o.requires
CMakeFiles/imgui.dir/requires: CMakeFiles/imgui.dir/resources/ocornut_imgui/imgui_gl3w.c.o.requires

.PHONY : CMakeFiles/imgui.dir/requires

CMakeFiles/imgui.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/imgui.dir/cmake_clean.cmake
.PHONY : CMakeFiles/imgui.dir/clean

CMakeFiles/imgui.dir/depend:
	cd /home/jb/Documents/Voraldo-v1.2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jb/Documents/Voraldo-v1.2 /home/jb/Documents/Voraldo-v1.2 /home/jb/Documents/Voraldo-v1.2 /home/jb/Documents/Voraldo-v1.2 /home/jb/Documents/Voraldo-v1.2/CMakeFiles/imgui.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/imgui.dir/depend

