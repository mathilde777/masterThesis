
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
#include <QCoreApplication>
#include <QObject>
#include <QMetaType>

TaskManager::TaskManager(std::shared_ptr<Database> db) : db(db) {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
    taskExecuting = false;
    donePreparing = false;
     conversionX = 6.50f;
     conversionY = 8.00f;
     conversionZ = 9.99f;
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

    connect(preparer, &TaskPreparer::finished, this, &TaskManager::preparingDone);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();


}


void TaskManager::preparingDone() {
    std::cout << "DONE PREPARING"<< std::endl;
    donePreparing = true; // Set the flag to true when the thread is finished preparing tasks
}

void TaskManager::trayDocked() {
     std::cout << "tray docked executing tasks"<< std::endl;
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
            emit updateStatus(QString("ADD : start to Add box with id %1").arg(task->getBoxId()));
            std::cout << "adding box" << std::endl;
            db->storeBox(task->getBoxId(), task->getTray());
        }

        else if(task->getType()==0)
        {
            findBoxesOfSameSize(*task->getBox());
             emit updateStatus(QString("FIND : start to find box with id %1").arg(task->getBoxId()));
            std::cout << "finding box" << std::endl;
            std::cout << "RUN 3D imaging" << std::endl;
                //cropping to match box prervius side
            //TODO: need check for if previous is 0,0,0 it means there never was an update and thus must take the whole image -> after testing

            Eigen::Vector3f lastPosition(task->getBox()->last_x, task->getBox()->last_y, task->getBox()->last_z);
            std::shared_ptr<std::vector<ClusterInfo>> resultsCluster;
            if(lastPosition == Eigen::Vector3f(0.0f, 0.0f, 0.0f))
            {
                emit updateStatus(QString("FIND ERROR : box wiht id %1 has no previous location -> scanning full tray").arg(task->getBoxId()));
                std::cout << "Last position: " << lastPosition << std::endl;
                resultsCluster = run3DDetection();
            }
            else
            {
                emit updateStatus(QString("FIND : running 3D").arg(task->getBoxId()));
                //Eigen::Vector3f dimensions(task->getBox()->width, task->getBox()->height, task->getBox()->length);


                std::cout << "Last position: " << lastPosition << std::endl;
               // std::cout << "Dimensions: " << dimensions << std::endl;
               resultsCluster = run3DDetection(lastPosition, task->getBox()->getClusterDimensions());
            }

            auto result = match_box(resultsCluster,task);
            std::cout << "Result: " << result << std::endl;

            if (result != Eigen::Vector3f(0.0f, 0.0f, 0.0f))
            {
                QString resultString = QString("%1, %2, %3").arg(result.x()).arg(result.y()).arg(result.z());
                emit updateStatus(QString("FIND Successful: box found at %1").arg(resultString));
                std::cout << "task completed time to remove stored box" << task->getBoxId() << std::endl;
                db->removeStoredBox(task->getBoxId());
                std::cout << "FOUND AT " << result << std::endl;
            }
            else

            {
                emit updateStatus(QString("FIND ERROR : box wiht id %i not found").arg(task->getBoxId()));
                std::cout << "BOX not found" << task->getBoxId() << std::endl;
                emit errorOccurredTask(QString("Box wiht id %1 not found ").arg(task->getBoxId()), task->getId());
                //update(task->getTray());
            }

        }
        db->removeTaskFromQueue(executingQueue.front()->getId());
        executingQueue.erase(executingQueue.begin());
        emit taskCompleted();
    }
}



//void TaskManager::match_box(std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results, std::shared_ptr<Task> task)
Eigen::Vector3f  TaskManager::match_box(std::shared_ptr<std::vector<ClusterInfo>> results, std::shared_ptr<Task> task)
{Eigen::Vector3f  result = Eigen::Vector3f();

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
        auto& centroid = results->begin()->centroid;
        if (centroid.size() >= 3) {
            result << centroid(0), centroid(1), centroid(2);  // Explicitly copy the first three components
        }
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
            auto directory = "/home/user/windows-share";
            auto PNGPath = photoProcessing->findLatestPngFile(directory);
            std::cout << "Path: " << PNGPath->c_str() << std::endl;

            //check if Path is correct , return string if not correct ==> Error
            if(!PNGPath){
                std::cout << "Error: No PNG file found" << std::endl;
                break;
            }

            // if (itn->dimensions.x() != 0 && itn->centroid.x() != 0 && itn->centroid.y() != 0 ){
            //     //iNtegrate Cropping
            //     photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());

            //     auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
            //     std::cout << "Crooped Path: " << PNGPathCropped->c_str() << std::endl;

            //     //check if Path is correct , return string if not correct ==> Error
            //     if(!PNGPathCropped){
            //         std::cout << "Error: No PNG file found" << std::endl;
            //         break;
            //     }
            //     else{
            //         std::cout << "Looking for box wiht id " <<  box->getBoxId() << std::endl;
            //     }
            //     //check with 2 D
            //     std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);
            // }

                //iNtegrate Cropping
            photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());

            auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
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
            std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);




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
    emit updateStatus("FIND ERROR : no result -> Scanning the whole tray");
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

bool TaskManager::compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2) {
    return boxPtr1->id < boxPtr2->id;
}



void TaskManager::sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    std::sort(trayBoxes.begin(), trayBoxes.end(), [this](const std::shared_ptr<Box>& a, const std::shared_ptr<Box>& b) {
        return this->compareBoxPtrByID(a, b);
    });

}


void TaskManager::update(int id)
{
    std::cout << "runing update" << std::endl;
    emit updateStatus(QString(" UPDATE : running"));
    bool error1 = false;
    bool error2 = false;
    trayBoxes.clear();
    trayBoxes = db->getAllBoxesInTray(id);
   sortTrayBoxesByID(trayBoxes);
    std::cout << "Number of boxes in tray: " << trayBoxes.size() << std::endl;
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
    std::cout << "cluster size" << resultsCluster->size() << std::endl;
    std::cout << "box size" << trayBoxes.size() << std::endl;
    if(resultsCluster->size() > trayBoxes.size())
    {
        emit updateStatus(QString("UPDATE ERROR : box added without infroming system or unknon box added"));
        std::cout << "EXTRA BOX FOUND" << std::endl;
        error1 = true;
        // box added
        // unkonw box
    }
    else if(resultsCluster->size() < trayBoxes.size())
    {
        emit updateStatus(QString("UPDATE ERROR : stacked boxes, box out of range and/or box removed without informing system"));
        std::cout << "not all boxes found" << std::endl;
        error2 = true;
        // stakced
        //out of range
        // box removed
    }
    for (const auto& cluster : *resultsCluster) {
        std::vector<shared_ptr<Box>> matchedBoxIds;
        for (const auto& box : trayBoxes) {
            if (dimensionsMatch(cluster, *box)) {
                matchedBoxIds.push_back(box);
                 std::cout << "Cluster " << cluster.clusterId << "matched to box"  <<box->getBoxId()<< std::endl;
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
                 std::cout << "Box  " <<box->getBoxId() <<"matched to cluster" << cluster.clusterId <<    std::endl;
            }

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
            db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(), matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
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
                auto directory = "/home/user/windows-share";
                auto PNGPath = photoProcessing->findLatestPngFile(directory);
                std::cout << "Path: " << PNGPath->c_str() << std::endl;

                //check if Path is correct , return string if not correct ==> Error
                if(!PNGPath){
                    std::cout << "Error: No PNG file found" << std::endl;
                    break;
                }
                //iNtegrate Cropping
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());

                auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
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

                db->updateBox(box->getId(),matchedBoxes[0].centroid.x(),matchedBoxes[0].centroid.y(),matchedBoxes[0].centroid.z(),matchedBoxes[0].dimensions.x(),matchedBoxes[0].dimensions.y(),matchedBoxes[0].dimensions.z());
                matchedCLuster->push_back(matchedBoxes[0]);
            }



        }

    //HNADEL ERROR BOXES
        std::shared_ptr<std::vector<ClusterInfo>> errorClusters = std::make_shared<std::vector<ClusterInfo>>();


        for ( const auto& cluster : *resultsCluster) {

            if (!isClusterAlreadyInList(cluster.clusterId, matchedCLuster)) {
                errorClusters->push_back(cluster);
            }
        }

    /////////////////////////////////NEW UPDATE FRO OPONLY LABEL BACK UP THE CONVERSION////////////////////////////////////////////////
    if(!errorBoxes.empty())
    {
        std::cout << "TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT "  << std::endl;

        auto it = errorClusters->begin();
        while (it != errorClusters->end()) {
            //Get latest png from PhotoProcessing
            auto directory = "/home/user/windows-share";
            auto PNGPath = photoProcessing->findLatestPngFile(directory);
            std::cout << "Path: " << PNGPath->c_str() << std::endl;

            //check if Path is correct, return string if not correct ==> Error
            if (!PNGPath) {
                std::cout << "Error: No PNG file found" << std::endl;
                break;
            }
            //Integrate Cropping
            photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());

            auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
            std::cout << "Crooped Path: " << PNGPathCropped->c_str() << std::endl;

            //check if Path is correct, return string if not correct ==> Error
            if (!PNGPathCropped) {
                std::cout << "Error: No PNG file found" << std::endl;
                break;
            }
            else {
                std::cout << "Looking for box with id " << std::endl;
            }
            //check with 2D
            std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);
            for (const auto res : *ret2) {
                std::cout << "2D Box: " << res.label << std::endl;
                it = errorClusters->erase(it);
                continue;
            }

            if (ret2->size() == 1) {
                std::cout << "size: " << errorBoxes.size() << std::endl;
                std::vector<std::shared_ptr<Box>> idk;

                for (auto it2 = errorBoxes.begin(); it2 != errorBoxes.end(); ++it2) {
                    std::cout << "Box: " << ret2->front().label << std::endl;
                    if (ret2->front().label == (*it2)->getBoxId()) {
                        // Erase the element from matchedBoxes
                        idk.push_back((*it2));
                        std::cout << "size: " << idk.size() << std::endl;
                        it2 = errorBoxes.erase(it2);
                    }

                    std::sort(idk.begin(), idk.end(), [this, &it](const std::shared_ptr<Box>& a, const std::shared_ptr<Box>& b) {
                        // Calculate distances between cluster centroids and the box
                        double distanceA = distance(a->last_x, a->last_y, a->last_z, it->centroid.x(), it->centroid.y(), it->centroid.z());
                        double distanceB = distance(b->last_x, b->last_y, b->last_z, it->centroid.x(), it->centroid.y(), it->centroid.z());

                        return distanceA < distanceB;
                    });

                    if (!idk.empty()) {
                        std::cout << "UPDATE!" << errorBoxes.size() << std::endl;
                        db->updateBox((*idk.begin())->getId(), it->centroid.x(), it->centroid.y(), it->centroid.z(), it->dimensions.x(), it->dimensions.y(), it->dimensions.z());
                        matchedCLuster->push_back(*it);
                        idk.erase(idk.begin());

                        if (!idk.empty()) {
                            errorBoxes.insert(errorBoxes.end(), idk.begin(), idk.end());
                        }
                    }
                }
            }
        }

    }

   /////// HANDLE OTHER ERRRORS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/////
    if(error1 || error2)
    {

        emit updateStatus(QString("ERROR  :  CHECK TRAY FIX AND RE RUN UPDATE"));
    }

    emit updateStatus(QString("UPDATE  :  done"));
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
    float threshold = 2.5; // Adjust as needed
    //auto conversion = 5.64634146;
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
        widthMatch = std::abs(cluster.dimensions.x()/conversionX  - std::get<0>(dim1)) < threshold;
        heightMatch = std::abs(cluster.dimensions.z()/(conversionZ) - std::get<2>(dim1)) < threshold;
        lengthMatch = std::abs(cluster.dimensions.y()/(conversionY) - std::get<1>(dim1)) < threshold;
        if(widthMatch && lengthMatch && heightMatch)          
        {
            std::cout << "DImension pair " << std::get<0>(dim1) << " : " << std::get<1>(dim1) << " : "  << std::get<2>(dim1) <<  std::endl;
            std::cout << "Dimensions of cluster: " << cluster.dimensions.x()/conversionX << " " << cluster.dimensions.y()/conversionY << " " << cluster.dimensions.z()/(conversionZ) << endl;
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

