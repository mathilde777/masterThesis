#include "findTask.h"
#include "task.h"
#include <iostream>

FindTask::FindTask() : TaskType() {}

void FindTask::executeTask(std::shared_ptr<Task> task) {
    // Implementation of find task
    std::cout << "Executing Find Task" << std::endl;
}

void FindTask::specificFunctionForFindTask() {
    // Implementation of a specific function for FindTask
}
