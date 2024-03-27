
#include "TaskManager.h"
#include "database.h"
#include "detection2D.h"

TaskManager::TaskManager(std::shared_ptr<Database> db) : db(db) {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);}



TaskManager::~TaskManager() {
    disconnect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);

}
void TaskManager::addTask(int boxId, int trayId, int task) {
    // Implementation for adding a task
}

bool sortByPriority(const std::unique_ptr<Task>& task1, const std::unique_ptr<Task>& task2) {
    if (task1->getType() != task2->getType())
        return task1->getType() < task2->getType();
    return task1->getId() < task2->getId();
}

void TaskManager::getTasks(int trayId)
{
    queue = db->getTasks(trayId);
    for (const auto& task : queue) {
        std::cout << "TASK: " << task->getId() << std::endl;
    }
    
    //sort is so the adds are first!!!!
    sort(queue.begin(), queue.end(), sortByPriority);
    for (const auto& task : queue) {
        std::cout << "TASK: " << task->getId() << std::endl;
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
        auto task = std::move(queue.front());
        if (task->getType() == 1) {
            std::cout << "adding box" << std::endl;
            db->storeBox(task->getBoxId(), task->getTray());

        }
        else if(task->getType()==0)
        {
            if(db->checkExistingBoxes(task->getTray(), task->getBoxId()))
            {
                //hre we starts 2 threads
                // decide how to cmomprae;
                std::cout << "RUN 2D imageing" << std::endl;
                //std::unique_ptr<DetectionResult> result = run2D("test4.jpeg");
               // std::cout << result->label << std::endl;
                std::cout << "RUN 3D imaging" << std::endl;

<<<<<<< HEAD
/**
                auto refPoint = Eigen::Vector3f(128, -269, -860.041);
=======
                // auto refPoint = Eigen::Vector3f(128, -269, -860.041);
>>>>>>> 695bf78639adc2d8b4b3a6df3800813859c2eaff

                // cout << "Reference point: " << refPoint.x() << " " << refPoint.y() << " " << refPoint.z() << endl;

<<<<<<< HEAD
                auto location = pcl->findBoundingBox(filePathBoxes, filePathEmpty, refPoint, Eigen::Vector3f(0, 0, 0));

                //location is type of clusterInfo ==> struct ClusterInfo { Eigen::Vector4f centroid, Eigen::Vector3f dimensions; Eigen::Quaternionf orientation; int clusterId;
                for (auto loc : location)
                {
                    cout << "Cluster ID: " << loc.clusterId << endl;
                    cout << "Centroid: " << loc.centroid.x() << " " << loc.centroid.y() << " " << loc.centroid.z() << endl;
                    cout << "Dimensions: " << loc.dimensions.x() << " " << loc.dimensions.y() << " " << loc.dimensions.z() << endl;
                    cout << "Orientation: " << loc.orientation.x() << " " << loc.orientation.y() << " " << loc.orientation.z() << " " << loc.orientation.w() << endl;
                }
**/
=======
                // auto location = pcl->findBoundingBox(filePathBoxes, filePathEmpty, refPoint, Eigen::Vector3f(0, 0, 0));
                // std::cout << "RUN 3D imaging" << std::endl;

                // //location is type of clusterInfo ==> struct ClusterInfo { Eigen::Vector4f centroid, Eigen::Vector3f dimensions; Eigen::Quaternionf orientation; int clusterId;
                // for (auto loc : location)
                // {
                //     cout << "Cluster ID: " << loc.clusterId << endl;
                //     cout << "Centroid: " << loc.centroid.x() << " " << loc.centroid.y() << " " << loc.centroid.z() << endl;
                //     cout << "Dimensions: " << loc.dimensions.x() << " " << loc.dimensions.y() << " " << loc.dimensions.z() << endl;
                //     cout << "Orientation: " << loc.orientation.x() << " " << loc.orientation.y() << " " << loc.orientation.z() << " " << loc.orientation.w() << endl;
                // }
>>>>>>> 695bf78639adc2d8b4b3a6df3800813859c2eaff
            }
            else
            {
                std::cout << "RUN 3D imaging" << std::endl;
            }

            std::cout << "task completed time to remove stored box"<< task->getBoxId()  << std::endl;
            db->removeStoredBox(task->getBoxId());

        }


        std::cout << "arrived at emit" << std::endl;
        emit onTaskCompleted();

    }
    else {
        std::cout << "All tasks executed." << std::endl;
    }
    return 0; // Placeholder return value
}

int TaskManager::onTaskCompleted() {

    db->removeTaskFromQueue(std::move(queue.front())->getId());

    queue.erase(queue.begin());
    emit refresh();
    for (const auto& task : queue) {
        std::cout << task->getId() << std::endl;
    }

    if (!queue.empty()) {

        std::cout << "arrive new execture" << std::endl;
        executeTasks();
    }
    else
    {
        std::cout << "UPDATE DATABASE - ALL TASKS DONE" << std::endl;
    }
    return 0; // Placeholder return value
}

int TaskManager::addBox(){
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
