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
CMAKE_SOURCE_DIR = /home/user/Documents/Thesis/simulation

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/user/Documents/Thesis/simulation/build

# Include any dependencies generated for this target.
include CMakeFiles/simulation.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/simulation.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/simulation.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/simulation.dir/flags.make

CMakeFiles/simulation.dir/main.cpp.o: CMakeFiles/simulation.dir/flags.make
CMakeFiles/simulation.dir/main.cpp.o: ../main.cpp
CMakeFiles/simulation.dir/main.cpp.o: CMakeFiles/simulation.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/Documents/Thesis/simulation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/simulation.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simulation.dir/main.cpp.o -MF CMakeFiles/simulation.dir/main.cpp.o.d -o CMakeFiles/simulation.dir/main.cpp.o -c /home/user/Documents/Thesis/simulation/main.cpp

CMakeFiles/simulation.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simulation.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/Documents/Thesis/simulation/main.cpp > CMakeFiles/simulation.dir/main.cpp.i

CMakeFiles/simulation.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simulation.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/Documents/Thesis/simulation/main.cpp -o CMakeFiles/simulation.dir/main.cpp.s

CMakeFiles/simulation.dir/mainwindow.cpp.o: CMakeFiles/simulation.dir/flags.make
CMakeFiles/simulation.dir/mainwindow.cpp.o: ../mainwindow.cpp
CMakeFiles/simulation.dir/mainwindow.cpp.o: CMakeFiles/simulation.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/Documents/Thesis/simulation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/simulation.dir/mainwindow.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simulation.dir/mainwindow.cpp.o -MF CMakeFiles/simulation.dir/mainwindow.cpp.o.d -o CMakeFiles/simulation.dir/mainwindow.cpp.o -c /home/user/Documents/Thesis/simulation/mainwindow.cpp

CMakeFiles/simulation.dir/mainwindow.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simulation.dir/mainwindow.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/Documents/Thesis/simulation/mainwindow.cpp > CMakeFiles/simulation.dir/mainwindow.cpp.i

CMakeFiles/simulation.dir/mainwindow.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simulation.dir/mainwindow.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/Documents/Thesis/simulation/mainwindow.cpp -o CMakeFiles/simulation.dir/mainwindow.cpp.s

CMakeFiles/simulation.dir/database.cpp.o: CMakeFiles/simulation.dir/flags.make
CMakeFiles/simulation.dir/database.cpp.o: ../database.cpp
CMakeFiles/simulation.dir/database.cpp.o: CMakeFiles/simulation.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/Documents/Thesis/simulation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/simulation.dir/database.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simulation.dir/database.cpp.o -MF CMakeFiles/simulation.dir/database.cpp.o.d -o CMakeFiles/simulation.dir/database.cpp.o -c /home/user/Documents/Thesis/simulation/database.cpp

CMakeFiles/simulation.dir/database.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simulation.dir/database.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/Documents/Thesis/simulation/database.cpp > CMakeFiles/simulation.dir/database.cpp.i

CMakeFiles/simulation.dir/database.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simulation.dir/database.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/Documents/Thesis/simulation/database.cpp -o CMakeFiles/simulation.dir/database.cpp.s

CMakeFiles/simulation.dir/addBox.cpp.o: CMakeFiles/simulation.dir/flags.make
CMakeFiles/simulation.dir/addBox.cpp.o: ../addBox.cpp
CMakeFiles/simulation.dir/addBox.cpp.o: CMakeFiles/simulation.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/Documents/Thesis/simulation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/simulation.dir/addBox.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simulation.dir/addBox.cpp.o -MF CMakeFiles/simulation.dir/addBox.cpp.o.d -o CMakeFiles/simulation.dir/addBox.cpp.o -c /home/user/Documents/Thesis/simulation/addBox.cpp

CMakeFiles/simulation.dir/addBox.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simulation.dir/addBox.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/Documents/Thesis/simulation/addBox.cpp > CMakeFiles/simulation.dir/addBox.cpp.i

CMakeFiles/simulation.dir/addBox.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simulation.dir/addBox.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/Documents/Thesis/simulation/addBox.cpp -o CMakeFiles/simulation.dir/addBox.cpp.s

CMakeFiles/simulation.dir/task.cpp.o: CMakeFiles/simulation.dir/flags.make
CMakeFiles/simulation.dir/task.cpp.o: ../task.cpp
CMakeFiles/simulation.dir/task.cpp.o: CMakeFiles/simulation.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/Documents/Thesis/simulation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/simulation.dir/task.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/simulation.dir/task.cpp.o -MF CMakeFiles/simulation.dir/task.cpp.o.d -o CMakeFiles/simulation.dir/task.cpp.o -c /home/user/Documents/Thesis/simulation/task.cpp

CMakeFiles/simulation.dir/task.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simulation.dir/task.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/Documents/Thesis/simulation/task.cpp > CMakeFiles/simulation.dir/task.cpp.i

CMakeFiles/simulation.dir/task.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simulation.dir/task.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/Documents/Thesis/simulation/task.cpp -o CMakeFiles/simulation.dir/task.cpp.s

# Object files for target simulation
simulation_OBJECTS = \
"CMakeFiles/simulation.dir/main.cpp.o" \
"CMakeFiles/simulation.dir/mainwindow.cpp.o" \
"CMakeFiles/simulation.dir/database.cpp.o" \
"CMakeFiles/simulation.dir/addBox.cpp.o" \
"CMakeFiles/simulation.dir/task.cpp.o"

# External object files for target simulation
simulation_EXTERNAL_OBJECTS =

simulation: CMakeFiles/simulation.dir/main.cpp.o
simulation: CMakeFiles/simulation.dir/mainwindow.cpp.o
simulation: CMakeFiles/simulation.dir/database.cpp.o
simulation: CMakeFiles/simulation.dir/addBox.cpp.o
simulation: CMakeFiles/simulation.dir/task.cpp.o
simulation: CMakeFiles/simulation.dir/build.make
simulation: /usr/lib/x86_64-linux-gnu/libmysqlcppconn.so
simulation: CMakeFiles/simulation.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/user/Documents/Thesis/simulation/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable simulation"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simulation.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/simulation.dir/build: simulation
.PHONY : CMakeFiles/simulation.dir/build

CMakeFiles/simulation.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/simulation.dir/cmake_clean.cmake
.PHONY : CMakeFiles/simulation.dir/clean

CMakeFiles/simulation.dir/depend:
	cd /home/user/Documents/Thesis/simulation/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/user/Documents/Thesis/simulation /home/user/Documents/Thesis/simulation /home/user/Documents/Thesis/simulation/build /home/user/Documents/Thesis/simulation/build /home/user/Documents/Thesis/simulation/build/CMakeFiles/simulation.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/simulation.dir/depend

