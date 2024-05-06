#include "taskPreparer.h"
#include "database.h"
#include "qtimer.h"
#include <iostream>
#include "box.h"

#include <QMetaType>
#include <memory>

TaskPreparer::TaskPreparer(int trayId, std::shared_ptr<Database> db) : trayId(trayId) ,db(db){
     std::cout << "test6"<< std::endl;

}
TaskPreparer::~TaskPreparer()

{

}
void TaskPreparer::prepareTask() {

    getTasks();

    for (const auto& task : tasks) {
        std::cout << "PREPING TASK: " << task->getId() << std::endl;
            emit taskPrepared(task);
    }
    emit finished();

}


void TaskPreparer::getTasks() {
    tasks = db->getTasks(trayId);
    for (const auto& task : tasks) {
        std::cout << "TASK: " << task->getId() << std::endl;
        if(task->task_type == 0)
        {
        // Fetch box information using the task's box ID
        auto box = db->getBox(trayId, task->getBoxId());
        if (box) {
            std::cout << "BOX: " << box->getId() << std::endl;
            task->setBox(box);
        } else {
            std::cerr << "Error: Unable to retrieve box information for task " << task->getId() << std::endl;
        }
        }
    }
}
