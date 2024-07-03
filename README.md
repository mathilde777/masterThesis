Welcome to our thesis called Localization of known objects to allow object picking
Project Overview
----------------
This project is a comprehensive implementation for managing various tasks related to detection and processing. The codebase is organized in C++ and includes functionalities for handling 2D and 3D detections, database interactions, and task management.

this code is run with 2 other sources:
- 3D localization library:https://github.com/sulikismaylovv/library_3D
- 2D object recognition system: https://github.com/sulikismaylovv/2D_detection
- MySQL database: see dump file provided

The projected is created and run in QT creator: https://www.qt.io/download-dev
## Requirements
### Software Dependencies
- **Qt6** (Core, Widgets, Sql)
- **PCL (Point Cloud Library)** (common, io, filters, visualization, segmentation, surface)
- **OpenCV**
- **MySQL Connector/C++:** reguired for the connection between the database and the algo. (https://dev.mysql.com/downloads/connector/cpp/ , https://dev.mysql.com/doc/connector-cpp/9.0/en/ , also see CMakeLists.txt)
- **libcurl**

Directory Structure
-------------------
* masterThesis-clean_up/
* 2dDetection.cpp
* AddTask.h
* Addtask.cpp
* BaseTask.h
* Box.h
* CMakeLists.txt
* CMakeLists.txt.user
* Database.cpp
* Database.h
* Detection2D.cpp
* Detection2D.h
* Detection3D.cpp
* Detection3D.h
* FindTask.cpp
* FindTask.h
* PhotoProcessing.h
* Result.h
* Task.h
* TaskFunctions.cpp
* TaskFunctions.h
* TaskManager.cpp
* TaskManager.h
* TaskPreparer.cpp
* TaskPreparer.h
* UpdateTask.cpp
* UpdateTask.h
* addTask.cpp
* functions.cpp
* .gitignore
* knownBox.h
* main.cpp
* mainwindow.cpp
* mainwindow.h
* mainwindow.ui
* simulation_en_150.ts
* task.cpp

File Descriptions
-----------------

1. AddTask.h / Addtask.cpp
   - Header and implementation for executing box additions.

2. BaseTask.h
   - Defines the base class for tasks.

3. Box.h
   - Header file for Box entity

4. Database.cpp / Database.h
   - Contains the logic for interacting with the database.

5. Detection2D.cpp / Detection2D.h
   - Header and source files for 2D detection functionalities.

6. Detection3D.cpp / Detection3D.h
   - Header and source files for 3D detection functionalities.

7. FindTask.cpp / FindTask.h
   - Logic for finding boxes.

8. PhotoProcessing.h
    -Photo processing tasks.

9. Result.h
    - Defines the structure for results.

10. Task.h
    - Defines the structure for tasks.
* The task types are as follows:
    *  0 -> find
    * 1-> add
    * 2 -> update

11. TaskFunctions.cpp / TaskFunctions.h
    - Contains various utility functions for task management. used by UpdateTask and FindTask

12. TaskManager.cpp / TaskManager.h
    - Manages all tasks of the tray

13. TaskPreparer.cpp / TaskPreparer.h
    - Prepares tasks before execution.

14. UpdateTask.cpp / UpdateTask.h
    - Logic for updating the tray.

15. knownBox.h
    -a type of box that is known. these are pulled form the database and are the boxes that can be added to trays

22. main.cpp
    - Entry point of the program.

23. mainwindow.cpp / mainwindow.h / mainwindow.ui
    - Implements the main window of the GUI.

  
