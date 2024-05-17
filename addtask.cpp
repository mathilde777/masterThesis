// add.cpp
#include "addTask.h"
#include "task.h"
#include <iostream>

AddTask::AddTask(std::shared_ptr<Task> task) : TaskType(task) {}

void AddTask::executeTask() {
    // Implementation of add task
    std::cout << "Executing Add Task" << std::endl;
}

void AddTask::specificFunctionForAddTask() {
    // Implementation of a specific function for AddTask
}
