#include "AddTask.h"
#include <iostream>

AddTask::AddTask(const std::shared_ptr<Task>& task) : task(task) {}

void AddTask::execute(std::shared_ptr<Database> db) {
    std::cout << "adding box" << std::endl;
    db->storeBox(task->getBoxId(), task->getTray());
    // Emit signals or handle post-addition logic as necessary
}
