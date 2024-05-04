cmake_minimum_required(VERSION 3.5)

project(simulation VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0144 NEW)


# Find Qt6
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(PCL 1.3 REQUIRED COMPONENTS common io filters visualization segmentation surface)
find_package(OpenCV REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})


#include_directories("/home/user/Documents/Thesis/library/lib/include")

#Suleyman:
include_directories("/home/Desktop/MasterThesis/library/lib/include")

set(MYSQLCPPCONN_INCLUDE_DIRS "/usr/include/mysql-cppconn-8/jdbc/cppconn/")
#set(MYSQLCPPCONN_LIBRARIES "/home/suleyman/Downloads/mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit/lib64/")

# Add your source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    database.h
    database.cpp
    addBox.cpp


    detection2D.h
    detection2D.cpp
    TaskManager.cpp
    TaskManager.h
    taskPreparer.h
    taskPreparer.cpp
    findCases.cpp
    findCases.h

    photoProcessing.h
    3D_detection.cpp
    result.h
    box.h
    task.h
    3D_detection.h
)

# Add an executable target
add_executable(simulation ${SOURCES}
    matchingLogic.cpp
    matchingLogic.h )

# Include libcurl headers
find_package(CURL REQUIRED)
target_include_directories(simulation PRIVATE ${CURL_INCLUDE_DIRS} ${MYSQLCPPCONN_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS} "/home/suleyman/Desktop/MasterThesis/library/lib/include" ${OpenCV_INCLUDE_DIRS})

# Link against Qt modules and MySQL Connector/C++
target_link_libraries(simulation PRIVATE Qt5::Widgets Qt5::Sql Qt5::Core
        ${OpenCV_LIBS}
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
