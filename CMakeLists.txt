cmake_minimum_required(VERSION 3.5)

project(simulation VERSION 0.1 LANGUAGES C CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically handle MOC, UIC, and RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(PCL 1.3 REQUIRED COMPONENTS common io filters visualization segmentation surface)
find_package(CURL REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})

include_directories("/home/Desktop/MasterThesis/library/lib/include")


# Manually set MySQL Connector/C++ include directories and libraries
set(MYSQLCPPCONN_INCLUDE_DIRS "/usr/include/mysql-cppconn-8")
#set(MYSQLCPPCONN_LIBRARIES "/home/suleyman/Downloads/mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit/lib64/")

# Specify the list of source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    database.h
    database.cpp
    addBox.cpp
    task.h
    task.cpp
    detection2D.h
    detection2D.cpp
    TaskManager.cpp
    TaskManager.h
)

# Define the executable target
add_executable(simulation ${SOURCES})

# Include directories
target_include_directories(simulation PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${MYSQLCPPCONN_INCLUDE_DIRS}
)

# Link libraries (Qt modules, MySQL Connector/C++, and libcurl)
target_link_libraries(simulation PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
    ${CURL_LIBRARIES}
    ${MYSQLCPPCONN_LIBRARIES}
    mysqlcppconn
    pthread
    ${PCL_LIBRARIES}
    "/home/suleyman/Desktop/MasterThesis/library/lib/lib3DSystem.so"
)

# Optimize build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler specific optimization flags
if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-O3 -march=native)
elseif(MSVC)
    add_compile_options(/O2)
endif()
