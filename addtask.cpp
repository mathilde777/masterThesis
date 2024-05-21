// add.cpp
#include "addTask.h"
#include "task.h"
#include <iostream>

AddTask::AddTask() : TaskType() {}

void AddTask::executeTask(std::shared_ptr<Task> task) {
    // Implementation of add task
    std::cout << "Executing Add Task" << std::endl;
}

void AddTask::specificFunctionForAddTask() {
    // Implementation of a specific function for AddTask
}
