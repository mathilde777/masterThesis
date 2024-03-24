cmake_minimum_required(VERSION 3.5)

project(simulation VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# Add your source files
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
)

# Add an executable target
add_executable(simulation ${SOURCES}
    TaskManager.cpp
    TaskManager.h

    )

# Include MySQL Connector/C++ headers
target_include_directories(simulation PRIVATE /home/user/Downloads/mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit/include)

# Link against MySQL Connector/C++ library
link_directories(/home/user/Downloads/mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit/lib64)

# Include libcurl headers
find_package(CURL REQUIRED)
target_include_directories(simulation PRIVATE ${CURL_INCLUDE_DIRS})

# Link against Qt modules and MySQL Connector/C++
target_link_libraries(simulation PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql mysqlcppconn ${CURL_LIBRARIES})

