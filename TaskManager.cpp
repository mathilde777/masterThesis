
#include "TaskManager.h"
#include "database.h"

TaskManager::TaskManager() {
}

TaskManager::~TaskManager() {

}
void TaskManager::addTask(int boxId, int trayId, int task) {
    // Implementation for adding a task
}

void TaskManager::getTasks(int trayId)
{
    queue = db.getTasks(trayId);
    for (const Task& task : queue) {
        std::cout << task.getId() << std::endl; // Assuming Task has a defined output operator
    }


}
void TaskManager::prepFirstFind()
{
    //here all preprocessing for the first add
}
void TaskManager::prepTasks(int id)
{
    getTasks(id);
}
void TaskManager::trayDocked() {
     std::cout << "tray docked executing tasks"<< std::endl;
     executeTasks();
}

int TaskManager::executeTasks() {
    if (!queue.empty()) {
        Task task = queue.front();
        if(task.getType() == 1)
        {
            std ::cout << "adding box" << std::endl;
            db.storeBox(task.getBoxId(),task.getTray());

        }
        else if(task.getType()==2)
        {
            if(db.checkExistingBoxes(task.getTray(), task.getBoxId()))
            {
                std::cout << "RUN 2D imageing" << std::endl;
                std::unique_ptr<DetectionResult> result = run2D("test4.jpeg");
                std::cout << result->label << std::endl;



                std::cout << "RUN 3D imaging" << std::endl;
            }
            else
            {
                std::cout << "RUN 3D imaging" << std::endl;
            }
            emit onTaskCompleted();

            //handling all the
        }
    } else {
        std::cout << "All tasks executed." << std::endl;
    }
    return 0; // Placeholder return value
}

int TaskManager::onTaskCompleted() {
    db.removeTaskFromQueue(queue.begin()->getId());
    queue.erase(queue.begin());
    if (!queue.empty()) {

        executeTasks();
    }
    else
    {
        std::cout << "UPDATE DATABASE - ALL TASKS DONE" << std::endl;
    }
    // Start executing the next task
    return 0; // Placeholder return value
}

int TaskManager::addBox() {
    // Implementation for adding a box
    return 0; // Placeholder return value
}

int TaskManager::findBox() {
    // Implementation for finding a box
    return 0; // Placeholder return value
}

void TaskManager::update()
{

}
