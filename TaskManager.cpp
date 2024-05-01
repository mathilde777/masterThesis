
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
    noResults = false;
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

                //cropping to match box prervius side
                //TODO: need check for if previous is 0,0,0 it means there never was an update and thus must take the whole image -> after testing
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
                    //Get latest png from PhotoProcessing
                    auto directory = "/home/suleyman/windows-share";
                    auto PNGPath = photoProcessing->findLatestPngFile(directory);
                    std::cout << "Path: " << PNGPath->c_str() << std::endl;

                    //check if Path is correct , return string if not correct ==> Error
                    if(!PNGPath)
                    {
                        std::cout << "Error: No PNG file found" << std::endl;
                    }
                    else
                    {
                        std::cout << "Looking for box wiht id " <<  task->getBoxId() << std::endl;

                        //check with 2 D
                        std::shared_ptr<std::vector<DetectionResult>> result2D = run2D(PNGPath->c_str(), 0);
                        for( const auto res : *result2D)
                        {
                            std::cout << "2D Boxx" << res.label << std::endl;
                        }
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
     putZeroLocationBoxesAtBack(trayBoxes);

    std::vector<std::shared_ptr<Box>> boxesToUpdate = db->getAllBoxesInTray(id);

    std::vector<std::pair<ClusterInfo, std::vector<shared_ptr<Box>>>> matches;
     std::vector<std::pair<shared_ptr<Box>, std::vector<ClusterInfo>>> matchesC;
    std::unordered_map<std::shared_ptr<ClusterInfo>, std::vector<int>> clusterToBoxes;
    std::unordered_map<int, std::vector<std::shared_ptr<ClusterInfo>>> boxToClusters;
    //THREADS

    //get lists
   // std::thread t1(run3DDetection(), "Hello");
   // std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection();
    std::cout << "starting 3d thread" << std::endl;
    std::future<std::shared_ptr<std::vector<ClusterInfo>>> ret = std::async([](){ return run3DDetection(); });
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = ret.get();

    //std::cout << "starting 2d thread" << std::endl;
    //std::future<std::shared_ptr<std::vector<DetectionResult>> > ret2 = std::async(run2D,"/home/user/Documents/Thesis/ModelsV3/ModelsV3/3box_center.png");
    //std::shared_ptr<std::vector<DetectionResult>> result2D = ret2.get();

    //list 3d
    //list 2d
    // list of boxes
    //comapres of the legnths of lists
    //if lagrger probelm
    //if smaller -> should be ok
    std::cout << "preping the matching in update" << std::endl;


    for (const auto& cluster : *resultsCluster) {
        std::vector<shared_ptr<Box>> matchedBoxIds;
        for (const auto& box : trayBoxes) {
            if (dimensionsMatch(cluster, *box)) {
                matchedBoxIds.push_back(box);
            }
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
        }
        matchesC.push_back(std::make_pair(box, matchedBoxIds));
    }
    std::shared_ptr<std::vector<ClusterInfo>> matchedCLuster = std::make_shared<std::vector<ClusterInfo>>();
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
            std::sort(matchedBoxes.begin(), matchedBoxes.end(), [&box, this](const ClusterInfo& a, const ClusterInfo& b) {
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
            //now we hvae a list we can use to consider teh previous locations

            //run 2d to get the label of that box.

           // std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D("/home/user/Documents/Thesis/ModelsV3/ModelsV3/3box_center.png", 1);
            // remove all the boxes taht dont have this product id from the list of matches box

            for(auto it = matchedBoxes.begin(); it != matchedBoxes.end(); )
            {
                //Get latest png from PhotoProcessing
                auto directory = "/home/suleyman/windows-share";
                auto PNGPath = photoProcessing->findLatestPngFile(directory);
                std::cout << "Path: " << PNGPath->c_str() << std::endl;

                //check if Path is correct , return string if not correct ==> Error
                if(!PNGPath)
                {
                    std::cout << "Error: No PNG file found" << std::endl;
                }
                else
                {

                    //check with 2 D
                    std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPath->c_str(), 0);
                    for( const auto res : *ret2)
                    {
                        std::cout << "2D Boxx" << res.label << std::endl;
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

                ++it; // Move to the next element
            }
            // take the first box

            db->updateBox(box->getId(),matchedBoxes[0].centroid.x(),matchedBoxes[0].centroid.y(),matchedBoxes[0].centroid.z());
            matchedCLuster->push_back(matchedBoxes[0]);


        }


      /**  auto it = std::find_if(boxesToUpdate.begin(), boxesToUpdate.end(),
                               [boxIdToRemove](const std::shared_ptr<Box>& boxPtr) {
                                   return boxPtr->getBoxId() == boxIdToRemove;
                               });

        // If the box with the given ID is found, erase it from the vector
        if (it != boxesToUpdate.end()) {
            boxesToUpdate.erase(it);
        }
**/
    }
/////////////////////////////////////////////////////////////
///
/**
    for(const auto& [cluster, matchedBoxes] : matches)
    {
        int boxIdToRemove;
        if(matchedBoxes.size() == 1)
        {
            std::cout << "UPDATING POSTION OF A BOXXXXXXXX" << std::endl;
            db->updateBox(matchedBoxes[0]->getId(),cluster.centroid.x(),cluster.centroid.y(),cluster.centroid.z());
            boxIdToRemove =  matchedBoxes[0]->getId();
        }
        else if(matchedBoxes.size() == 0)
        {
            std::cout << "ERROR: UN RECOGNIZEABLE BOX!" << std::endl;
        }
        else if(matchedBoxes.size() > 1)
        {
            // firstly we sort the array so to know which box as teh closets previous location: handy for later
            std::sort(matches.begin(), matches.end(), [this](const auto& a, const auto& b) {
                const ClusterInfo& clusterA = a.first;
                const ClusterInfo& clusterB = b.first;

                // Calculate distances between cluster centroids and last x, y, z of the boxes
                double distanceA = distance(clusterA.centroid.x(), clusterA.centroid.y(), clusterA.centroid.z(),
                                            a.second.back()->last_x, a.second.back()->last_y, a.second.back()->last_z);
                double distanceB = distance(clusterB.centroid.x(), clusterB.centroid.y(), clusterB.centroid.z(),
                                            b.second.back()->last_x, b.second.back()->last_y, b.second.back()->last_z);

                return distanceA < distanceB;
            });

            //now we hvae a list we can use to consider teh previous locations

            //run 2d to get the label of that box.

            // remove all the boxes taht dont have this product id from the list of matches box

            // take the first box
            boxIdToRemove =  1;



        }


        auto it = std::find_if(boxesToUpdate.begin(), boxesToUpdate.end(),
                               [boxIdToRemove](const std::shared_ptr<Box>& boxPtr) {
                                   return boxPtr->getBoxId() == boxIdToRemove;
                               });

        // If the box with the given ID is found, erase it from the vector
        if (it != boxesToUpdate.end()) {
            boxesToUpdate.erase(it);
        }
    }
**/
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
void TaskManager::deleteClusterById(std::shared_ptr<std::vector<ClusterInfo>> resultsCluster, int id) {
    auto& clusters = *resultsCluster; // Dereference the shared_ptr to get the vector

    // Iterate through the vector to find the cluster with the given ID
    auto it = std::remove_if(clusters.begin(), clusters.end(), [id](const ClusterInfo& cluster) {
        return cluster.clusterId == id;
    });

    // Erase the cluster from the vector
    clusters.erase(it, clusters.end());
}

void TaskManager::putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    // Partition the vector such that boxes with last_x, last_y, and last_z equal to 0 are placed at the end
    auto partitionIter = std::partition(trayBoxes.begin(), trayBoxes.end(), [](const std::shared_ptr<Box>& boxPtr) {
        const Box& box = *boxPtr;
        return box.last_x != 0 || box.last_y != 0 || box.last_z != 0;
    });
}


bool TaskManager::isClusterAlreadyInList(int clusterId, const std::shared_ptr<std::vector<ClusterInfo>>& clusters) {
    if (!clusters) {
        return false; // If the pointer is null, return false
    }

    // Iterate through the vector to check if a cluster with the same ID exists
    for (const auto& cluster : *clusters) {
        if (cluster.clusterId == clusterId) {
            return true; // Found a cluster with the same ID
        }
    }

    return false; // No cluster with the same ID found
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

void TaskManager::match_box(std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results, std::shared_ptr<Task> task)

{
    switch(results->size()) {
        //CASE: no resutls found: could indicate error or jsut hthat the box moved
    case 0: {
        if(noResults)
        {
              std::cout << "ERROR CANNOT FIND BOX " << std::endl;
        }
        else
        {
              handleNoResults(task);
        }
        break;
    }
        //CASE: 1 box found-> ideal case
    case 1: {
        db->updateBox(task->getBoxId(), results->begin()->first.centroid.x(), results->begin()->first.centroid.y(), results->begin()->first.centroid.z());
        break;
    }
        //CASE: mulitple boxes found -> multiple options
    default: {
        //check with 2 D
        std::cout << "Looking for box wiht id " <<  task->getBoxId() << std::endl;

        for(std::shared_ptr<Box> box_id : possibleSameSize ){
            std::cout << "But alos these boxes are possible" << box_id->getBoxId() << std::endl;
        }

        std::shared_ptr<std::vector<DetectionResult>> result2D = run2D("/home/user/Documents/Thesis/ModelsV3/ModelsV3/kinect_brown_top.png", 0);

        for (auto it = result2D->begin(); it != result2D->end();) {
            if ((*it).label != task->getBox()->getBoxId()) {
                it = result2D->erase(it);
            } else {
                ++it;
            }
        }
        switch(result2D->size()) {
            //CASE: no resutls found: could indicate error or jsut hthat the box moved
        case 0: {

            break;
        }
        //CASE: 1 box found-> ideal case
        case 1: {
            break;

        }
        //CASE: mulitple boxes found -> multiple options
        default: {

            break;
        }
        }
        break;
    }
    }
}
void TaskManager::handleNoResults(std::shared_ptr<Task> task) {
    noResults = true;
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster2 = run3DDetection();
    std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results2 = matchClusterWithBox(resultsCluster2, task->getBox());
    match_box(results2, task);
}
