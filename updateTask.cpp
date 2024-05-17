#include "updateTask.h"
#include <iostream>

UpdateTask::UpdateTask(std::shared_ptr<Task> task) : TaskType(task){

}

void UpdateTask::executeTask() {
    // Implementation of update task
    std::cout << "Executing Update Task" << std::endl;
}

void UpdateTask::specificFunctionForUpdateTask() {
    // Implementation of a specific function for UpdateTask
}
