#include "findTask.h"
#include "task.h"
#include <iostream>

FindTask::FindTask(std::shared_ptr<Task> task) : TaskType(task) {}

void FindTask::executeTask() {
    // Implementation of find task
    std::cout << "Executing Find Task" << std::endl;
}

void FindTask::specificFunctionForFindTask() {
    // Implementation of a specific function for FindTask
}
