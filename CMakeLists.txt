cmake_minimum_required(VERSION 3.5)

project(simulation VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# Find PCL
find_package(PCL 1.3 REQUIRED COMPONENTS common io filters visualization segmentation surface)

# Find OpenCV
find_package(OpenCV REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})


include_directories("/home/user/Documents/Thesis/library/lib/include")

# Add your source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Database.h
    Database.cpp
    Detection2D.h
    Detection2D.cpp
    TaskManager.cpp
    TaskManager.h
    TaskPreparer.h
    TaskPreparer.cpp
    PhotoProcessing.h
    Detection3D.cpp
    Result.h
    Box.h
    Task.h
    Detection3D.h
    knownBox.h
    BaseTask.h
    Addtask.cpp
    AddTask.h
    UpdateTask.h
    FindTask.h
    UpdateTask.cpp
    FindTask.cpp
    TaskFunctions.h
    TaskFunctions.cpp

)

# Add an executable target
add_executable(simulation ${SOURCES}






)

# Include MySQL Connector/C++ headers
target_include_directories(simulation PRIVATE /home/user/Downloads/mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit/include ${OpenCV_INCLUDE_DIRS})

# Link against MySQL Connector/C++ library
link_directories(/home/user/Downloads/mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit/lib64)

# Include libcurl headers
find_package(CURL REQUIRED)
target_include_directories(simulation PRIVATE ${CURL_INCLUDE_DIRS})

# Link against Qt modules and MySQL Connector/C++
target_link_libraries(simulation PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql
    mysqlcppconn
    ${CURL_LIBRARIES}
    ${PCL_LIBRARIES}
      ${OpenCV_LIBS}
    /home/user/Documents/Thesis/library/lib/lib3DSystem.so)
