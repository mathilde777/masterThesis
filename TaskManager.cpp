
#include "TaskManager.h"
#include "database.h"
#include "detection2D.h"
#include "qeventloop.h"
#include "taskPreparer.h"
#include <QTimer>
#include <memory>
#include "3D_detection.h"
#include "result.h"
#include <future>
#include <Eigen/Core>
#include <vector>    // for std::vector
#include <algorithm>
#include <iostream>


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
    QThread *thread = new QThread;
    tray = id;
    trayBoxes = db->getAllBoxesInTray(id);
    TaskPreparer *preparer = new TaskPreparer(id,db);

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
    noResults = false;
    if ( !taskExecuting) {
        auto task = executingQueue.front();
        taskExecuting = true;
        if (task->getType() == 1) {
            std::cout << "adding box" << std::endl;
            db->storeBox(task->getBoxId(), task->getTray());
        }

        else if(task->getType()==0)
        {
            findBoxesOfSameSize(*task->getBox());
            std::cout << "finding box" << std::endl;
            std::cout << "RUN 3D imaging" << std::endl;
            Eigen::Vector3f result;
                //cropping to match box prervius side
                //TODO: need check for if previous is 0,0,0 it means there never was an update and thus must take the whole image -> after testing
            Eigen::Vector3f lastPosititon;
            lastPosititon << task->getBox()->last_x,task->getBox()->last_y, task->getBox()->last_z;
            Eigen::Vector3f dimensions;
            dimensions << task->getBox()->width,task->getBox()->height, task->getBox()->length;

            std::cout << "Last position: " << lastPosititon << std::endl;
            std::cout << "Dimensions: " << dimensions << std::endl;
            std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection(lastPosititon, dimensions);
            result = match_box(resultsCluster,task);

            if (result != Eigen::Vector3f(0.0f, 0.0f, 0.0f))
            {
                std::cout << "task completed time to remove stored box" << task->getBoxId() << std::endl;
                db->removeStoredBox(task->getBoxId());
                std::cout << "FOUND AT " << result << std::endl;
            }
            else

            {
                std::cout << "BOX not found" << task->getBoxId() << std::endl;
                update(task->getTray());
            }

        }

        db->removeTaskFromQueue(executingQueue.front()->getId());
        executingQueue.erase(executingQueue.begin());
        emit taskCompleted();
    }
}



//void TaskManager::match_box(std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results, std::shared_ptr<Task> task)
Eigen::Vector3f  TaskManager::match_box(std::shared_ptr<std::vector<ClusterInfo>> results, std::shared_ptr<Task> task)
{Eigen::Vector3f  result = Eigen::Vector3f(0.0f, 0.0f, 0.0f);

    for(auto it = results->begin(); it != results->end();)
    {
        if (!dimensionsMatch(*it,*task->getBox()))
        {it = results->erase(it);
         continue;
        }
        ++it;
    }
    switch(results->size()) {
    case 0: {
        if(noResults)
        {
            std::cout << "ERROR CANNOT FIND BOX " << std::endl;
            break;
        }
        else
        {
            result = handleNoResults(task);
        }
        break;
    }
    case 1: {
        result << results->begin()->centroid;
        break;
    }
    default: {
        std::cout << "Looking for box wiht id " <<  task->getBoxId() << std::endl;

        for(std::shared_ptr<Box> box_id : possibleSameSize ){
            std::cout << "But alos these boxes are possible" << box_id->getBoxId() << std::endl;
        }
        auto box = task->getBox();
        std::sort(results->begin(), results->end(), [box , this](const ClusterInfo &a, const ClusterInfo &b) {
            // Calculate distances between cluster centroids and the box
            double distanceA = distance(a.centroid.x(), a.centroid.y(), a.centroid.z(),
                                        box->last_x, box->last_y, box->last_z);
            double distanceB = distance(b.centroid.x(), b.centroid.y(), b.centroid.z(),
                                        box->last_x, box->last_y, box->last_z);
            return distanceA < distanceB;
        });


        for(auto itn = results->begin(); itn != results->end(); )
        {
            //iNtegrate Cropping
            //Get latest png from PhotoProcessing
            auto directory = "/home/suleyman/windows-share";
            auto PNGPath = photoProcessing->findLatestPngFile(directory);
            std::cout << "Path: " << PNGPath->c_str() << std::endl;

            //check if Path is correct , return string if not correct ==> Error
            if(!PNGPath){
                std::cout << "Error: No PNG file found" << std::endl;
                break;
            }
            //iNtegrate Cropping
            photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());

            auto PNGPathCropped = photoProcessing->findLatestPngFile(directory);
            std::cout << "Crooped Path: " << PNGPathCropped->c_str() << std::endl;

            //check if Path is correct , return string if not correct ==> Error
            if(!PNGPathCropped){
                std::cout << "Error: No PNG file found" << std::endl;
                break;
            }
            else{
                std::cout << "Looking for box wiht id " <<  box->getBoxId() << std::endl;
            }
            //check with 2 D
            std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 0);
            if(!res2D->empty())
            {
                if (res2D->begin()->label != task->getBox()->getBoxId()) {
                    std::cout << "BOX type" << res2D->begin()->label << std::endl;
                    itn = results->erase(itn);
                } else {
                    ++itn;
                }
            }
            else
            {
                break;
            }
        }

        for (auto clusterA : *results)
        {
            std::cout << "x " << clusterA.centroid.x()<< "y " <<  clusterA.centroid.y()  << "z " <<  clusterA.centroid.z()<<std::endl;
        }
        if(!results->empty())
        {
            std::cout << "GOR TO FIND A BOXXXXXXXXXXXXXXXXXXXXXXXXXXXx "  << std::endl;
            result << results->begin()->centroid.x(),  results->begin()->centroid.y() , results->begin()->centroid.z();
            std::cout << result  << std::endl;
            break;
        }
        else
        {
            if(noResults)
            {
                std::cout << "ERROR CANNOT FIND BOX " << std::endl;

                break;
            }

            else{
                result = handleNoResults(task);
            }
        }
    }
    }
    return result;

}
Eigen::Vector3f TaskManager::handleNoResults(std::shared_ptr<Task> task) {
    noResults = true;
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster2 = run3DDetection();
    return match_box(resultsCluster2, task);
}
int TaskManager::run3DDetectionThread() {

    auto result = run3DDetection();

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

// Define a comparison function to compare IDs through shared pointers
// Define a comparison function to compare IDs through shared pointers
bool TaskManager::compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2) {
    return boxPtr1->id < boxPtr2->id;
}

// Sort function
void TaskManager::sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    std::sort(trayBoxes.begin(), trayBoxes.end(), [this](const std::shared_ptr<Box>& a, const std::shared_ptr<Box>& b) {
        return this->compareBoxPtrByID(a, b);
    });

}


void TaskManager::update(int id)
{
    std::cout << "runing update" << std::endl;
    bool error1 = false;
    bool error2 = false;
    trayBoxes.clear();
    trayBoxes = db->getAllBoxesInTray(id);
    sortTrayBoxesByID(trayBoxes);
    putZeroLocationBoxesAtBack(trayBoxes);
    for(auto box : trayBoxes)
    {
        std::cout << "box"<< box->getId() << std::endl;
    }

    std::vector<std::shared_ptr<Box>> errorBoxes = std::vector<std::shared_ptr<Box>>();

    std::vector<std::pair<ClusterInfo, std::vector<shared_ptr<Box>>>> matches;
    std::vector<std::pair<shared_ptr<Box>, std::vector<ClusterInfo>>> matchesC;

    std::shared_ptr<std::vector<ClusterInfo>> matchedCLuster = std::make_shared<std::vector<ClusterInfo>>();

   // std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection();
    std::cout << "starting 3d thread" << std::endl;
    std::future<std::shared_ptr<std::vector<ClusterInfo>>> ret = std::async([](){ return run3DDetection(); });
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = ret.get();
    std::cout << "preping the matching in update" << std::endl;

    if(resultsCluster->size() > trayBoxes.size())
    {
        std::cout << "EXTRA BOX FOUND" << std::endl;
        error1 = true;
    }
    else if(resultsCluster->size() < trayBoxes.size())
    {
        std::cout << "not all boxes found" << std::endl;
        error2 = true;
    }
    for (const auto& cluster : *resultsCluster) {
        std::vector<shared_ptr<Box>> matchedBoxIds;
        for (const auto& box : trayBoxes) {
            if (dimensionsMatch(cluster, *box)) {
                matchedBoxIds.push_back(box);
            }
            std::cout << "Cluster " << cluster.clusterId << "matched to box"  <<box->getBoxId()<< std::endl;
        }
        matches.push_back(std::make_pair(cluster, matchedBoxIds));
    }
    for (const auto& box : trayBoxes)
    {
        std::vector<ClusterInfo> matchedBoxIds;
        for (const auto& cluster : *resultsCluster) {
            if (dimensionsMatch(cluster, *box)) {
                matchedBoxIds.push_back(cluster);
            }
            std::cout << "Box  " <<box->getBoxId() <<"matched to cluster" << cluster.clusterId <<    std::endl;
        }
        matchesC.push_back(std::make_pair(box, matchedBoxIds));
    }


   //////////////////////////////////
    for (auto it = matchesC.begin(); it != matchesC.end(); ++it) {
        auto& box = it->first;
        auto& matchedBoxes = it->second;

        int boxIdToRemove;
        if (matchedBoxes.size() == 1) {
            std::cout << "UPDATING POSITION OF A BOX" << std::endl;
            db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z());
        } else if (matchedBoxes.size() == 0) {
            std::cout << "ERROR: UNRECOGNIZABLE BOX!" << std::endl;
            errorBoxes.push_back(box);
        } else if (matchedBoxes.size() > 1) {


            for(auto its = matchedBoxes.begin(); its != matchedBoxes.end(); )
            {

                if (isClusterAlreadyInList(its->clusterId,matchedCLuster))
                {
                        its = matchedBoxes.erase(its);
                }
                else{
                    ++its; // Move to the next element
                }

            }
            // Sort the matchedBoxes vector based on distance to the box
            std::sort(matchedBoxes.begin(), matchedBoxes.end(), [&box, this](const ClusterInfo &a, const ClusterInfo &b) {
                // Calculate distances between cluster centroids and the box
                double distanceA = distance(a.centroid.x(), a.centroid.y(), a.centroid.z(),
                                            box->last_x, box->last_y, box->last_z);
                double distanceB = distance(b.centroid.x(), b.centroid.y(), b.centroid.z(),
                                            box->last_x, box->last_y, box->last_z);

                return distanceA < distanceB;
            });
             std::cout << " BOX x " << box->getLastX() << "y " <<  box->getLastY() << "z " <<  box->getLastZ() <<std::endl;
            for (auto clusterA : matchedBoxes)
            {
                std::cout << "x " << clusterA.centroid.x()<< "y " <<  clusterA.centroid.y()  << "z " <<  clusterA.centroid.z()<<std::endl;
            }

            for(auto it = matchedBoxes.begin(); it != matchedBoxes.end(); )
            {
                //Get latest png from PhotoProcessing
                auto directory = "/home/suleyman/windows-share";
                auto PNGPath = photoProcessing->findLatestPngFile(directory);
                std::cout << "Path: " << PNGPath->c_str() << std::endl;

                //check if Path is correct , return string if not correct ==> Error
                if(!PNGPath){
                    std::cout << "Error: No PNG file found" << std::endl;
                    break;
                }
                //iNtegrate Cropping
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());

                auto PNGPathCropped = photoProcessing->findLatestPngFile(directory);
                std::cout << "Crooped Path: " << PNGPathCropped->c_str() << std::endl;

                //check if Path is correct , return string if not correct ==> Error
                if(!PNGPathCropped){
                    std::cout << "Error: No PNG file found" << std::endl;
                    break;
                }
                else{
                    std::cout << "Looking for box wiht id " <<  box->getBoxId() << std::endl;
                }
                //check with 2 D
                std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);
                for( const auto res : *ret2)
                {
                        std::cout << "2D Boxx" << res.label << std::endl;
                        it = matchedBoxes.erase(it);
                        continue;
                    }

                    if (ret2->size() == 1)
                    {
                        if (ret2->front().label != box->getBoxId())
                        {
                            // Erase the element from matchedBoxes
                            it = matchedBoxes.erase(it);
                            continue; // Continue to the next iteration without incrementing it
                        }
                    }

                }

                ++it;
            }
            if(matchedBoxes.empty())
            {
                std::cout << "ERROR: UNRECOGNIZABLE BOX!" << std::endl;
                errorBoxes.push_back(box);
            }
            else
            {

                db->updateBox(box->getId(),matchedBoxes[0].centroid.x(),matchedBoxes[0].centroid.y(),matchedBoxes[0].centroid.z());
                matchedCLuster->push_back(matchedBoxes[0]);
            }



        }

    //HNADEL ERROR BOXES

    if(!errorBoxes.empty())
    {
        std::shared_ptr<std::vector<ClusterInfo>> errorClusters = std::make_shared<std::vector<ClusterInfo>>();
        if(matchesC.size() != matchedCLuster->size())
        {

            for ( const auto& cluster : *resultsCluster) {

                    if (isClusterAlreadyInList(cluster.clusterId, matchedCLuster)) {
                    errorClusters->push_back(cluster);
                    }
            }
        }
    }
}


void TaskManager::deleteClusterById(std::shared_ptr<std::vector<ClusterInfo>> resultsCluster, int id) {
    auto& clusters = *resultsCluster;

    auto it = std::remove_if(clusters.begin(), clusters.end(), [id](const ClusterInfo &cluster) {
        return cluster.clusterId == id;
    });
    clusters.erase(it, clusters.end());
}

void TaskManager::putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    auto partitionIter = std::partition(trayBoxes.begin(), trayBoxes.end(), [](const std::shared_ptr<Box>& boxPtr) {
        const Box& box = *boxPtr;
        return box.last_x != 0 || box.last_y != 0 || box.last_z != 0;
    });
}


bool TaskManager::isClusterAlreadyInList(int clusterId, const std::shared_ptr<std::vector<ClusterInfo>>& clusters) {
    if (!clusters) {
        return false;
    }
    for (const auto& cluster : *clusters) {
        if (cluster.clusterId == clusterId) {
            return true;
        }
    }

    return false;
}

double TaskManager::distance(double x1, double y1, double z1, double x2, double y2, double z2) {
    return std::sqrt((x2 - x1) * (x2 - x1) +
                     (y2 - y1) * (y2 - y1) +
                     (z2 - z1) * (z2 - z1));
}

bool TaskManager::compareClosestToClusterCenter(const std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>& a,
                                                const std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>& b) {
    const ClusterInfo& clusterA = a.first;
    const ClusterInfo& clusterB = b.first;

    // Calculate distances between cluster centroids and last x, y, z of the boxes
    double distanceA = distance(clusterA.centroid.x(), clusterA.centroid.y(), clusterA.centroid.z(),
                                a.second.back()->last_x, a.second.back()->last_y, a.second.back()->last_z);
    double distanceB = distance(clusterB.centroid.x(), clusterB.centroid.y(), clusterB.centroid.z(),
                                b.second.back()->last_x, b.second.back()->last_y, b.second.back()->last_z);

    return distanceA < distanceB;
}

bool TaskManager::dimensionsMatch(const ClusterInfo &cluster, const Box &box1) {
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
void TaskManager::findBoxesOfSameSize(const Box &box1)
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

