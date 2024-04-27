
#include "TaskManager.h"
#include "database.h"
#include "detection2D.h"
#include "qeventloop.h"
#include "qtmetamacros.h"
#include "taskPreparer.h"
#include <QTimer>
#include <memory>
#include "3D_detection.h"
#include "result.h"
#include <future>
#include <Eigen/Core>


TaskManager::TaskManager(std::shared_ptr<Database> db) : db(db) {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
    taskExecuting = false;
    donePreparing = false;
}



TaskManager::~TaskManager() {
    disconnect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
}


void TaskManager::prepTasks(int id)
{
    std::cout << "about to make the tasks"<< std::endl;
    QThread* thread = new QThread;
    tray = id;
    trayBoxes = db->getAllBoxesInTray(id);
    TaskPreparer* preparer = new TaskPreparer(id,db);

    preparer->moveToThread(thread);

    connect(preparer, &TaskPreparer::taskPrepared, this, &TaskManager::onTaskPrepared);
    connect(thread, &QThread::started, preparer, &::TaskPreparer::prepareTask);

    connect(preparer, &TaskPreparer::finished, this, &TaskManager::preparingDone); // Corrected connection

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();


}


void TaskManager::preparingDone() {
    std::cout << "DONE PREPARING"<< std::endl;
    donePreparing = true; // Set the flag to true when the thread is finished preparing tasks
}

void TaskManager::trayDocked() {
     std::cout << "tray docked executing tasks"<< std::endl;
     //executeTasks();
     startExecutionLoop();
     emit trayDockedUpdate();
}

void TaskManager::executeTasks() {

    bool success = false;
    if ( !taskExecuting) {
        auto task = executingQueue.front();
         std::cout << "crash?" << std::endl;
        taskExecuting = true;
         std::cout << "crash?" << std::endl;
        if (task->getType() == 1) {
            std::cout << "adding box" << std::endl;
            db->storeBox(task->getBoxId(), task->getTray());
            update(task->trayId);
        }

        else if(task->getType()==0)
        {
            findBoxesOfSameSize(*task->getBox());
             std::cout << "finding box" << std::endl;
                std::cout << "RUN 3D imaging" << std::endl;

             Eigen::Vector3f lastPosititon;
                lastPosititon << task->getBox()->getLastX(),task->getBox()->getLastY(), task->getBox()->getLastZ();
             Eigen::Vector3f dimensions;
             lastPosititon << task->getBox()->getWidth(),task->getBox()->getLength(), task->getBox()->getWidth();

             std::cout << "Last position: " << lastPosititon << std::endl;
             std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection(lastPosititon, dimensions);
                std::cout << "back in task manager" << std::endl;
                std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results = matchClusterWithBox(resultsCluster, task->getBox());
                if(results->size() > 1)
                {
                   //check with 2 D
                   std::shared_ptr<std::vector<DetectionResult>> result2D = run2D("/home/user/Documents/Thesis/ModelsV3/ModelsV3/3box_center.png");
                    for( const auto res : *result2D)
                   {
                       std::cout << "2D Boxx" << res.label << std::endl;
                   }
                }
               else if (results->empty())
               {
                    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection();
                    std::cout << "back in task manager" << std::endl;
                    std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results = matchClusterWithBox(resultsCluster, task->getBox());
               }

                else if(results->size() == 1)
               {
                    db->updateBox(task->getBoxId(),results->begin()->first.centroid.x(),results->begin()->first.centroid.y(),results->begin()->first.centroid.z());
               }
            std::cout << "task completed time to remove stored box" << task->getBoxId() << std::endl;
            db->removeStoredBox(task->getBoxId());
        }

        db->removeTaskFromQueue(executingQueue.front()->getId());
        executingQueue.erase(executingQueue.begin());
        emit taskCompleted();
    }


}
int TaskManager::run3DDetectionThread() {

    auto result = run3DDetection(); // Assuming run3DDetection returns a Result

}

void TaskManager::onTaskPrepared(std::shared_ptr<Task> task) {
    executingQueue.push_back(task);
     std::cout << "task preped and added to execute" << std::endl;
    emit taskPrepared();
}

void TaskManager::startExecutionLoop() {
      std::cout << "huh?" << std::endl;
    while (!executingQueue.empty() || !donePreparing) {
        if (!executingQueue.empty()) {
              std::cout << "huh2" << std::endl;
            executeTasks();
        } else if (!donePreparing) {
            waitForTasks();
        }
    }
    update(tray);
     std::cout << "DONE EXECUTING TASKS" << std::endl;
}

void TaskManager::waitForTasks() {
     std::cout << "waiting for TASKS" << std::endl;
    QEventLoop loop;
    connect(this, &::TaskManager::taskPrepared, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(this, &::TaskManager::taskPrepared, &loop, &QEventLoop::quit);
}

void TaskManager::onTaskCompleted() {
     std::cout << "task done" << std::endl;
    emit refresh();
    taskExecuting = false;
}

void TaskManager::update(int id)
{
    std::cout << "runing update" << std::endl;
    trayBoxes.clear();
    trayBoxes = db->getAllBoxesInTray(id);

    std::vector<std::pair<ClusterInfo, std::vector<int>>> matches;
    std::unordered_map<std::shared_ptr<ClusterInfo>, std::vector<int>> clusterToBoxes;
    std::unordered_map<int, std::vector<std::shared_ptr<ClusterInfo>>> boxToClusters;
    //THREADS

    //get lists
   // std::thread t1(run3DDetection(), "Hello");
   // std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection();
    std::cout << "starting 3d thread" << std::endl;
    std::future<std::shared_ptr<std::vector<ClusterInfo>>> ret = std::async([](){ return run3DDetection(); });
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = ret.get();

    std::cout << "starting 2d thread" << std::endl;
    std::future<std::shared_ptr<std::vector<DetectionResult>> > ret2 = std::async(run2D,"/home/user/Documents/Thesis/ModelsV3/ModelsV3/3box_center.png");
    std::shared_ptr<std::vector<DetectionResult>> result2D = ret2.get();

    //list 3d
    //list 2d
    // list of boxes
    //comapres of the legnths of lists
    //if lagrger probelm
    //if smaller -> should be ok
     std::cout << "preping the matching in update" << std::endl;
    for (const auto& cluster : *resultsCluster) {
        std::vector<int> matchedBoxIds;
        for (const auto& box : trayBoxes) {
            if (dimensionsMatch(cluster, *box)) {
                matchedBoxIds.push_back(box->id);
            }
        }
        matches.push_back(std::make_pair(cluster, matchedBoxIds));
    }

    for(const auto& [cluster, matchedBoxes] : matches)
    {
        if(matchedBoxes.size() == 1)
        {
            std::cout << "UPDATING POSTION OF A BOXXXXXXXX" << std::endl;
            db->updateBox(matchedBoxes[0],cluster.centroid.x(),cluster.centroid.y(),cluster.centroid.z());
        }
    }
    // so now we have a list of matches with the 3d
    for (const auto& [cluster, matchedBoxes] : clusterToBoxes) {
        std::cout << "Cluster ID: " << cluster->clusterId << std::endl;
        if (matchedBoxes.size() > 1) {
            std::cout << "Multiple box IDs: ";
            for (int id : matchedBoxes) {
                std::cout << id << " ";
            }
            std::cout << std::endl;
        } else if (matchedBoxes.empty()) {
            std::cout << "No matching boxes found" << std::endl;
        }
    }

    for (const auto& [boxId, matchedClusters] : boxToClusters) {
        std::cout << "Box ID: " << boxId << std::endl;
        if (matchedClusters.size() > 1) {
            std::cout << "Belongs to multiple clusters: ";
            for (const auto& cluster : matchedClusters) {
                std::cout << cluster->clusterId << " ";
            }
            std::cout << std::endl;
        } else if (matchedClusters.empty()) {
            std::cout << "Not assigned to any cluster" << std::endl;
        }
    }
}


bool TaskManager::dimensionsMatch(const ClusterInfo& cluster, const Box& box1) {
    // Define a threshold for matching dimensions
    std::cout << "DIMENSION MATHCINGr" << std::endl;
    float threshold = 2.0; // Adjust as needed
    auto conversion = 5.64634146;
    std::vector<std::tuple<double, double, double>> dimensionPairs = {
        {box1.width, box1.height, box1.length},
        {box1.width, box1.length, box1.height},
        {box1.height, box1.width, box1.length},
        {box1.height, box1.length, box1.width},
        {box1.length, box1.width, box1.height},
        {box1.length, box1.height, box1.width}
    };
    bool widthMatch = false;
    bool heightMatch = false;
    bool lengthMatch = false;
    bool match = false;
    for (const auto& dim1 : dimensionPairs) {
        widthMatch = std::abs(cluster.dimensions.x()/conversion - std::get<0>(dim1)) < threshold;
        heightMatch = std::abs(cluster.dimensions.z()/(conversion*1.5) - std::get<2>(dim1)) < threshold;
        lengthMatch = std::abs(cluster.dimensions.y()/(conversion) - std::get<1>(dim1)) < threshold;
        if(widthMatch && lengthMatch && heightMatch)          
        {
            std::cout << "DImension pair " << std::get<0>(dim1) << " : " << std::get<1>(dim1) << " : "  << std::get<2>(dim1) <<  std::endl;
            std::cout << "Dimensions of cluster: " << cluster.dimensions.x()/conversion << " " << cluster.dimensions.y()/conversion << " " << cluster.dimensions.z()/(conversion*1.5) << endl;
            match = true;
            break;
        }
    }

    return match;
}
void TaskManager::findBoxesOfSameSize(const Box& box1)
{


    std::vector<std::tuple<double, double, double>> dimensionPairs1 = {
        {box1.width, box1.height, box1.length},
        {box1.width, box1.length, box1.height},
        {box1.height, box1.width, box1.length},
        {box1.height, box1.length, box1.width},
        {box1.length, box1.width, box1.height},
        {box1.length, box1.height, box1.width}
    };
    for (const auto& box : trayBoxes) {
            std::vector<std::tuple<double, double, double>> dimensionPairs2 = {
                {box->width, box->height, box->length},
                {box->width, box->length, box->height},
                {box->height, box->width, box->length},
                {box->height, box->length, box->width},
                {box->length, box->width, box->height},
                {box->length, box->height, box->width}
            };
            bool foundMatch = false;
            if(box->boxId == box1.getBoxId())
            {
                foundMatch = true;
            }
            else{
            for (const auto& dim1 : dimensionPairs1) {
                for (const auto& dim2 : dimensionPairs2) {
                    if (dim1 == dim2 ) {
                        foundMatch = true;
                        break;
                    }
                }
                if (foundMatch) break;
            }
            }
            if (foundMatch){
                possibleSameSize.push_back(box);
            }
        }

}

