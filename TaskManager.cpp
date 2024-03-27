
#include "TaskManager.h"
#include "database.h"

TaskManager::TaskManager() {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
}

TaskManager::~TaskManager() {
    disconnect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);

}
void TaskManager::addTask(int boxId, int trayId, int task) {
    // Implementation for adding a task
}

bool sortByPriority(const Task& task1, const Task& task2) {
    if (task1.task_type != task2.task_type)
        return task1.task_type < task2.task_type;
    return task1.id < task2.id;
}
void TaskManager::getTasks(int trayId)
{
     Database db = Database();
    queue = db.getTasks(trayId);

    for (const Task& task : queue) {
        std::cout << task.getId() << std::endl; 
    }
    
    //sort is so the adds are first!!!!
    sort(queue.begin(), queue.end(), sortByPriority);  
    for (const Task& task : queue) {
        std::cout << task.getId() << std::endl; 
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
            Database db = Database();
            db.storeBox(task.getBoxId(),task.getTray());

        }
        else if(task.getType()==0)
        {
            Database db = Database();
            if(db.checkExistingBoxes(task.getTray(), task.getBoxId()))
            {
                //hre we starts 2 threads
                // decide how to cmomprae;
                std::cout << "RUN 2D imageing" << std::endl;
                std::unique_ptr<DetectionResult> result = run2D("test4.jpeg");
                std::cout << result->label << std::endl;



                std::cout << "RUN 3D imaging" << std::endl;
            }
            else
            {
                std::cout << "RUN 3D imaging" << std::endl;
            }

            std::cout << "task completed time to remove stored box"<< task.getBoxId()  << std::endl;
            db.removeStoredBox(task.getBoxId());

        }


        std::cout << "arrived at emit" << std::endl;
        emit onTaskCompleted();

    }
    else {
        std::cout << "All tasks executed." << std::endl;
    }
    return 0; // Placeholder return value
}

void TaskManager::onTaskCompleted() {
     Database db = Database();
    std::cout << "arrived at complete" << std::endl;
    db.removeTaskFromQueue(queue.begin()->getId());
    queue.erase(queue.begin());
    emit refresh();
    for (const Task& task : queue) {
        std::cout << task.getId() << std::endl;
    }

    if (!queue.empty()) {

        std::cout << "arrive new execture" << std::endl;
        executeTasks();
    }
    else
    {
        std::cout << "UPDATE DATABASE - ALL TASKS DONE" << std::endl;
    }
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
