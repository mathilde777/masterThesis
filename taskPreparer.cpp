#include "taskPreparer.h"
#include "database.h"
#include "qtimer.h"
#include "qtmetamacros.h"
#include <iostream>
#include "box.h"

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


void TaskPreparer::getTasks()
{
    tasks  = db->getTasks(trayId);
    for (const auto& task : tasks) {
        std::cout << "TASK: " << task->getId() << std::endl;
    }



}
