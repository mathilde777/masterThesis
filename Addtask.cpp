#include "AddTask.h"
#include <iostream>

AddTask::AddTask(std::shared_ptr<Database> db): db(db){}

void AddTask::execute(const std::shared_ptr<Task>& task) {
    this->task = task;
    std::cout << "adding box" << std::endl;
    db->storeBox(task->getBoxId(), task->getTray());
}
