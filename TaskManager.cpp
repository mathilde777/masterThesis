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

TaskManager::TaskManager(std::shared_ptr<Database> db, std::shared_ptr<std::vector<std::shared_ptr<KnownBox> > > knownBoxes) : db(db), knownBoxes(knownBoxes) {
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
    connect(preparer, &TaskPreparer::finished, this, &TaskManager::preparingDone);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();

}

int TaskManager::checkFlaggedBoxes(int productId)
{
    for(const auto& box: *knownBoxes)
    {
        if(box->productId == productId)
        {
            return box->trained;
        }
   }
    return 0;
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
void TaskManager::onTaskPrepared(std::shared_ptr<Task> task) {
    executingQueue.push_back(task);
    std::cout << "task preped and added to execute" << std::endl;
    emit taskPrepared();
}

void TaskManager::startExecutionLoop() {
    while (!executingQueue.empty() || !donePreparing) {
        if (!executingQueue.empty()) {
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
void TaskManager::executeTasks() {
    noResults = false;
    if (taskExecuting || executingQueue.empty()) {
        return; // Early return if a task is already executing or the queue is empty
    }

    auto task = executingQueue.front();
    taskExecuting = true;

    switch (task->getType()) {
    case 1:
        executeAddBoxTask(task);
        break;
    case 0:
        executeFindBoxTask(task);
        break;
    default:
        std::cout << "UNKOWN TASK "<< std::endl;
        break;
    }
}

void TaskManager::executeAddBoxTask(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("ADD : start to Add box with id %1").arg(task->getBoxId()));
    std::cout << "adding box" << std::endl;
    db->storeBox(task->getBoxId(), task->getTray());
    removeExecutedTask(task);
}

void TaskManager::executeFindBoxTask(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("FIND : start to find box with id %1").arg(task->getBoxId()));
    std::cout << "finding box" << std::endl;
    std::cout << "RUN 3D imaging" << std::endl;

    Eigen::Vector3f lastPosition(task->getBox()->last_x, task->getBox()->last_y, task->getBox()->last_z);
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster;
    if (lastPosition == Eigen::Vector3f(0.0f, 0.0f, 0.0f)) {
        executeFullTrayScan(task);
    } else {
        executePartialTrayScan(task, lastPosition);
    }

}

void TaskManager::executeFullTrayScan(const std::shared_ptr<Task>& task) {
    std::cout << "FULL TRAY SCAN " << std::endl;

    emit updateStatus(QString("FIND ERROR : box with id %1 has no previous location -> scanning full tray").arg(task->getBoxId()));
    auto resultsCluster = run3DDetection();
    processBoxDetectionResult(task, resultsCluster);
}

void TaskManager::executePartialTrayScan(const std::shared_ptr<Task>& task, const Eigen::Vector3f& lastPosition) {
     std::cout << "PARTIAL TRAY SCAN " << std::endl;
    emit updateStatus(QString("FIND : running 3D for %1").arg(task->getBoxId()));
    std::cout << "Last position: " << lastPosition << std::endl;
    auto resultsCluster = ::run3DDetection(lastPosition, task->getBox()->getClusterDimensions());
    processBoxDetectionResult(task, resultsCluster);
}

void TaskManager::processBoxDetectionResult(const std::shared_ptr<Task>& task, const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    auto result = match_box(resultsCluster, task);
    std::cout << "Result: " << result << std::endl;

    if (result != Eigen::Vector3f(0.0f,0.0f,0.0f)) {
        handleSuccessfulBoxFound(task, result);
        removeExecutedTask(task);
    } else {
        if(noResults)
        {
            handleFailedBoxDetection(task);
            removeExecutedTask(task);
        }
        else
        {
            executeFullTrayScan(task);
        }
    }
}

void TaskManager::handleSuccessfulBoxFound(const std::shared_ptr<Task>& task, const Eigen::Vector3f& result) {
    QString resultString = QString("%1, %2, %3").arg(result.x()).arg(result.y()).arg(result.z());
    emit updateStatus(QString("FIND Successful: box found at %1").arg(resultString));
    std::cout << "task completed time to remove stored box" << task->getBoxId() << std::endl;
    db->removeStoredBox(task->getBoxId());
    std::cout << "FOUND AT " << result << std::endl;
}

void TaskManager::handleFailedBoxDetection(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("FIND ERROR : box with id %1 not found").arg(task->getBoxId()));
    std::cout << "BOX not found" << task->getBoxId() << std::endl;
    emit errorOccurredTask(QString("Box with id %1 not found ").arg(task->getBoxId()), task->getId());
    //update(task->getTray());
}

void TaskManager::removeExecutedTask(const std::shared_ptr<Task>& task) {
    db->removeTaskFromQueue(task->getId());
    executingQueue.pop_front(); // Remove the task from the queue
    emit taskCompleted();
}



Eigen::Vector3f  TaskManager::match_box(std::shared_ptr<std::vector<ClusterInfo>> results, std::shared_ptr<Task> task)
{Eigen::Vector3f  result = Eigen::Vector3f(0.0f,0.0f,0.0f);

    // Null pointer check
    if (!results || results->empty() || !task) {
        std::cerr << "ERROR: Invalid input data." << std::endl;
        return result;
    }

    // Filter results based on dimensions
    auto it = std::remove_if(results->begin(), results->end(), [&](const ClusterInfo& info) {
        return !dimensionsMatch(info, *task->getBox());
    });
    results->erase(it, results->end());

    switch(results->size()) {
    case 0: {
        result = Eigen::Vector3f(0.0f,0.0f,0.0f);
        if(!noResults)
        {
            noResults = true;
        }
    }
    case 1: {

        auto itn = results->begin();
        const auto& centroid = itn->centroid;
            auto PNGPath = photoProcessing->findLatestPngFile("/home/userwindows-share");
            if (!PNGPath) {
                std::cerr << "ERROR: No PNG file found" << std::endl;
                break;
            }

            // photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());
            //Box is sided
            if( itn->dimensions.x() < itn->dimensions.z() || itn->dimensions.y() < itn->dimensions.z()){
                if (itn->dimensions.x() > itn->dimensions.y()){
                    photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.y(), itn->dimensions.x());
                }
                else{
                    photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());
                }
            }

            else{
                if (itn->dimensions.x() > itn->dimensions.y()){
                    photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.y(), itn->dimensions.x());
                }
                else{
                    photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());
                }
            }

            auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
            if (!PNGPathCropped) {
                std::cerr << "ERROR: No cropped PNG file found" << std::endl;
                break;
            }

            std::cout << "Looking for box with type ID: " << task->getBox()->getBoxId() << std::endl;
            std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);
            if(checkFlaggedBoxes(task->getBox()->getBoxId()))
            {
                if (!res2D->empty()) {
                    if (res2D->front().label != task->getBox()->getBoxId()) {

                    }
                    else {
                        if (centroid.size() >= 3) {
                            result << centroid(0), centroid(1), centroid(2);
                        } else {
                            std::cerr << "ERROR: Invalid centroid data." << std::endl;
                        }
                        break;
                    }
                }
                else {
                    break;
                }
            }
            else

            {
                 std::cout << "NOT TRAINED BOX IN FIND:" << task->getBox()->getBoxId() << std::endl;
                //save image
                if (centroid.size() >= 3) {
                    result << centroid(0), centroid(1), centroid(2);
                } else {
                    std::cerr << "ERROR: Invalid centroid data." << std::endl;
                }
                break;
            }



    }
    default: {
        std::cout << "Looking for box with ID: " << task->getBoxId() << std::endl;
        for (const auto& box_id : possibleSameSize) {
            std::cout << "Also these boxes are possible: " << box_id->getBoxId() << std::endl;
        }
        auto box = task->getBox();
        sortResultsByDistance(results, task->getBox());

        for (auto itn = results->begin(); itn != results->end();) {

            auto PNGPath = photoProcessing->findLatestPngFile("/home/userwindows-share");
            if (!PNGPath) {
                std::cerr << "ERROR: No PNG file found" << std::endl;
                break;
            }

           // photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(), itn->dimensions.x(), itn->dimensions.y());
            //Box is sided
            if( it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()){
                if (it->dimensions.x() > it->dimensions.y()){
                    photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
                }
                else{
                    photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
                }
            }

            else{
                if (it->dimensions.x() > it->dimensions.y()){
                    photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
                }
                else{
                    photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
                }
            }

            auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
            if (!PNGPathCropped) {
                std::cerr << "ERROR: No cropped PNG file found" << std::endl;
                break;
            }

            std::cout << "Looking for box with ID: " << box->getBoxId() << std::endl;
            std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);

            if(checkFlaggedBoxes(task->getBox()->getBoxId()))
            {
                if (!res2D->empty()) {
                    if (res2D->front().label != task->getBox()->getBoxId()) {
                        std::cout << "BOX type" << res2D->front().label << std::endl;
                        itn = results->erase(itn);
                    } else {
                        ++itn;
                    }
                } else {
                    break;
                }
            }
            else

            {
                //save image
                //no need to do anyhting else, we jsut take the first value
            }

        }

        for (const auto& clusterA : *results) {
            std::cout << "x " << clusterA.centroid.x() << "y " << clusterA.centroid.y() << "z " << clusterA.centroid.z() << std::endl;
        }

        if (!results->empty()) {
            const auto& centroid = results->front().centroid;
            if (centroid.size() >= 3) {
                result << centroid(0), centroid(1), centroid(2);
                std::cout << result << std::endl;
            }
        }
        else {
            result = Eigen::Vector3f(0.0f,0.0f,0.0f);
            if(!noResults)
            {
                noResults = true;
            }
            break;
        }
        break;
    }
    }
    return result;

}
Eigen::Vector3f TaskManager::handleNoResults(std::shared_ptr<Task> task) {
    noResults = true;
    emit updateStatus("FIND ERROR : no result -> Scanning the whole tray");
    executeFullTrayScan( task);
    //std::shared_ptr<std::vector<ClusterInfo>> resultsCluster2 = run3DDetection();
    //return match_box(resultsCluster2, task);
}
int TaskManager::run3DDetectionThread() {

    auto result = run3DDetection();

}
double distance(double x1, double y1, double z1, double x2, double y2, double z2) {
    return std::sqrt((x2 - x1) * (x2 - x1) +
                     (y2 - y1) * (y2 - y1) +
                     (z2 - z1) * (z2 - z1));
}
bool sortByDistance(const ClusterInfo& a, const ClusterInfo& b, const std::shared_ptr<Box>& box) {
    double distanceA = distance(a.centroid.x(), a.centroid.y(), a.centroid.z(),
                                box->last_x, box->last_y, box->last_z);
    double distanceB = distance(b.centroid.x(), b.centroid.y(), b.centroid.z(),
                                box->last_x, box->last_y, box->last_z);
    return distanceA < distanceB;
}

void TaskManager::sortResultsByDistance(std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Box>& box) {
    std::sort(results->begin(), results->end(), [box](const ClusterInfo& a, const ClusterInfo& b) {
        return sortByDistance(a, b, box);
    });
}
bool TaskManager::compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2) {
    return boxPtr1->id < boxPtr2->id;
}



void TaskManager::sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    std::sort(trayBoxes.begin(), trayBoxes.end(), [this](const std::shared_ptr<Box>& a, const std::shared_ptr<Box>& b) {
        return this->compareBoxPtrByID(a, b);
    });

}

void TaskManager::update(int id) {
    std::cout << "Running update" << std::endl;
    emit updateStatus(QString(" UPDATE : running"));

    trayBoxes.clear();
    trayBoxes = db->getAllBoxesInTray(id);
    sortTrayBoxesByID(trayBoxes);
    matchedCluster->clear();
    errorClusters->clear();
    errorBoxes.clear();
    std::cout << "Number of boxes in tray: " << trayBoxes.size() << std::endl;
    putZeroLocationBoxesAtBack(trayBoxes);

    std::future<std::shared_ptr<std::vector<ClusterInfo>>> resultFuture = std::async([this]() {
        return run3DDetection();
    });
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = resultFuture.get();

    // Error checking
    bool error1 = checkForExtraBoxes(trayBoxes, resultsCluster);
    bool error2 = checkForMissingBoxes(trayBoxes, resultsCluster);

    // Matching clusters to boxes
    std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> matches = matchClustersToBoxes(trayBoxes, resultsCluster);

    // Matching boxes to clusters
    std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> matchesC = matchBoxesToClusters(trayBoxes, resultsCluster);

    // Handling matched clusters and updating box information
    for (auto& match : matchesC) {
        handleMatchedBoxes(match.first, match.second);
        for ( const auto& cluster : *matchedCluster) {

            std::cout << "cluster in matched" << cluster.clusterId << std::endl;
        }
    }

    std::cout << "macthed bioxes " << matchedCluster->size() << std::endl;
    for ( const auto& cluster : *matchedCluster) {

        std::cout << "cluster in matched" << cluster.clusterId << std::endl;
    }
    for ( const auto& cluster : *resultsCluster) {

          std::cout << "cluster in results " << cluster.clusterId << std::endl;
        if (!isClusterAlreadyInList(cluster.clusterId)) {
            errorClusters->push_back(cluster);
        }
    }
    // Handling error boxes
    if(!errorBoxes.empty())
    {
        handleErrorBoxes();

    }

    // Handling other errors
    handleOtherErrors(error1, error2);

    emit updateStatus(QString("UPDATE  :  done"));
}
bool TaskManager::checkForExtraBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                                     const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    return trayBoxes.size() < resultsCluster->size();
}

bool TaskManager::checkForMissingBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                                       const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    return trayBoxes.size() > resultsCluster->size();
}
std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> TaskManager::matchClustersToBoxes(
    const std::vector<std::shared_ptr<Box>>& trayBoxes,
    const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> matches;
    std::cout << "CLUSTERS TO BOXES " <<    std::endl;
    for (const auto& cluster : *resultsCluster) {
        std::vector<std::shared_ptr<Box>> matchedBoxes;
        for (const auto& box : trayBoxes) {
            if (dimensionsMatch(cluster, *box)) {
                matchedBoxes.push_back(box);
            }
        }

        matches.emplace_back(cluster, matchedBoxes);

        std::cout << "Cluster  "<< cluster.clusterId  <<" matched to boxes "<<    std::endl;
        std::cout << " " << std::endl;
        for (const auto& box : matchedBoxes) {
            std::cout << "box  " << box->getBoxId() <<    std::endl;
        }
        std::cout << "------------------------------------" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << " " << std::endl;
    }
    return matches;
}

std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> TaskManager::matchBoxesToClusters(
    const std::vector<std::shared_ptr<Box>>& trayBoxes,
    const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> matches;
    std::cout << "BOX TO CLUSTERS " <<    std::endl;
    for (const auto& box : trayBoxes) {
        std::vector<ClusterInfo> matchedClusters;
        for (const auto& cluster : *resultsCluster) {
            if (dimensionsMatch(cluster, *box)) {
                matchedClusters.push_back(cluster);

            }
        }
        matches.emplace_back(box, matchedClusters);
        std::cout << "Box  " <<box->getBoxId() <<" matched to clusters "<<    std::endl;
        std::cout << " " << std::endl;
        for (const auto& cluster : matchedClusters) {
            std::cout << "cluster  " <<cluster.clusterId <<    std::endl;
        }
        std::cout << "------------------------------------" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << " " << std::endl;
    }

    return matches;
}
void TaskManager::handleMatchedBoxes(const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes) {
    if (matchedBoxes.empty()) {
        std::cout << "ERROR: UNRECOGNIZABLE BOX!" << std::endl;
        errorBoxes.push_back(box);
        return;
    }
    else if(matchedBoxes.size() == 1)
    {

        std::cout << "UPDATING POSITION OF A BOX" << std::endl;
        auto it = matchedBoxes.begin();
        if(isClusterAlreadyInList(it->clusterId))
        {
           errorBoxes.push_back(box);
            return;
        }

        auto directory = "/home/user/windows-share";
        auto PNGPath = photoProcessing->findLatestPngFile(directory);
        std::cout << "Path: " << (PNGPath ? PNGPath->c_str() : "Error: No PNG file found") << std::endl;

        if (!PNGPath) {
            std::cout << "Error: No PNG file found" << std::endl;

        }
       //Box is sided
        if( it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()){
            if (it->dimensions.x() > it->dimensions.y()){
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            }
            else{
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            }
        }

        else{
            if (it->dimensions.x() > it->dimensions.y()){
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            }
            else{
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            }
        }


        auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
        std::cout << "Crooped Path: " << (PNGPathCropped ? PNGPathCropped->c_str() : "Error: No PNG file found") << std::endl;

        if (!PNGPathCropped) {
            std::cout << "Error: No PNG file found" << std::endl;

        } else {
            std::cout << "Looking for box with id " << box->getBoxId() << std::endl;
        }

        // Check with 2D

        std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);

        if(checkFlaggedBoxes(box->getBoxId()))
        {
            for (const auto& res : *ret2) {
                std::cout << "2D Boxx" << res.label << std::endl;
            }

            if ( ret2->front().label != box->getBoxId()) {
                return;
            }
            else {

                db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(), matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
                matchedCluster->push_back(matchedBoxes[0]);
                return;
            }
        }
        else
        {
            ///save image????
            db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(), matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
            matchedCluster->push_back(matchedBoxes[0]);
            return;
        }

     }

    // Remove matched clusters that are already in the list
    auto it = std::remove_if(matchedBoxes.begin(), matchedBoxes.end(), [&](const ClusterInfo& cluster) {
        return isClusterAlreadyInList(cluster.clusterId);
    });
    matchedBoxes.erase(it, matchedBoxes.end());

    // Sort matched clusters based on distance
    std::sort(matchedBoxes.begin(), matchedBoxes.end(), [&](const ClusterInfo& a, const ClusterInfo& b) {
        return sortByDistance(a, b, box);
    });

    // Sort results by distance
    sortResultsByDistance(matchedCluster, box);

    std::cout << "BOX x " << box->getLastX() << " y " << box->getLastY() << " z " << box->getLastZ() << std::endl;
    for (const auto& cluster : matchedBoxes) {
        std::cout << "Cluster x " << cluster.centroid.x() << " y " << cluster.centroid.y() << " z " << cluster.centroid.z() << std::endl;
    }

    for (auto it = matchedBoxes.begin(); it != matchedBoxes.end(); ) {
        std::cout << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << std::endl;
        std::cout << "Cluster: " << it->clusterId << std::endl;
        std::cout << "Cluster: x " << it->centroid.x() << " y " << it->centroid.y() << " z " << it->centroid.z() << std::endl;

        // Get latest png from PhotoProcessing
        auto directory = "/home/user/windows-share";
        auto PNGPath = photoProcessing->findLatestPngFile(directory);
        std::cout << "Path: " << (PNGPath ? PNGPath->c_str() : "Error: No PNG file found") << std::endl;

        if (!PNGPath) {
            std::cout << "Error: No PNG file found" << std::endl;
            break;
        }


        //Box is sided
        if( it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()){
            if (it->dimensions.x() > it->dimensions.y()){
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            }
            else{
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            }
        }

        else{
            if (it->dimensions.x() > it->dimensions.y()){
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            }
            else{
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            }
        }


        auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
        std::cout << "Crooped Path: " << (PNGPathCropped ? PNGPathCropped->c_str() : "Error: No PNG file found") << std::endl;

        if (!PNGPathCropped) {
            std::cout << "Error: No PNG file found" << std::endl;
            break;
        } else {
            std::cout << "Looking for box with id " << box->getBoxId() << std::endl;
        }

        // Check with 2D

        std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);

        if(checkFlaggedBoxes(box->getBoxId()))
        {
            for (const auto& res : *ret2) {
                std::cout << "2D Boxx" << res.label << std::endl;
            }

            if ( ret2->front().label != box->getBoxId()) {
                it = matchedBoxes.erase(it);
            } else {
                ++it;
            }
        }

    }

    if (matchedBoxes.empty()) {
        std::cout << "ERROR: UNRECOGNIZABLE BOX - NO MATCHING AFTER SORTING" << std::endl;
        errorBoxes.push_back(box);
        return;
    } else {
        db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(), matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
        matchedCluster->push_back(matchedBoxes[0]);
        return;
    }
}
void TaskManager::handleErrorBoxes() {
    //Line sepaarator
    std::cout << "HANDELING ERROR BOXES" << std::endl;
    std::cout << " " << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << " " << std::endl;

    // Print error boxes
    for (const auto& box : errorBoxes) {
        std::cout << "Error Box: " << box->getBoxId() << std::endl;
    }
    std::cout << "errorLCuster size" << errorClusters->size() << std::endl;
    auto it = errorClusters->begin();
    while (it != errorClusters->end()) {
        std::cout << "HANDELING CLUSTER WITH id "<< it->clusterId << std::endl;
        //Print which cluster is being checked
        std::cout << "Cluster: " << it->clusterId << std::endl;
        std::cout << "x " << it->centroid.x()<< "y " <<  it->centroid.y()  << "z " <<  it->centroid.z()<<std::endl;



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
        //Box is sided
        if( it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()){
            photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
        }
        else{
            photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
        }

        auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
        std::cout << "Crooped Path: " << PNGPathCropped->c_str() << std::endl;

        //check if Path is correct, return string if not correct ==> Error
        if (!PNGPathCropped) {
            std::cout << "Error: No PNG file found" << std::endl;
            break;
        }
        else {
            //std::cout << "Looking for box with id " << std::endl;
        }
        //check with 2D
        std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);

        //Print detection result
        for (const auto res : *ret2) {
            std::cout << "2D Box Prediction: " << res.label << std::endl;
        }

        std::vector<std::shared_ptr<Box>> idk;

        if (ret2->size() == 1) {
            std::cout << "size of Error Boxes: " << errorBoxes.size() << std::endl;

            for (auto it2 = errorBoxes.begin(); it2 != errorBoxes.end();) {
                std::cout << "Box: " << ret2->front().label << std::endl;
                if (ret2->front().label == (*it2)->getBoxId()) {
                    // Erase the element from matchedBoxes
                    idk.push_back((*it2));
                    std::cout << "size: " << idk.size() << std::endl;
                    it2 = errorBoxes.erase(it2);
                }
                else{
                    ++it2;
                }
            }

            std::sort(idk.begin(), idk.end(), [this, &it](const std::shared_ptr<Box>& a, const std::shared_ptr<Box>& b) {
                // Calculate distances between cluster centroids and the box
                double distanceA = distance(a->last_x, a->last_y, a->last_z, it->centroid.x(), it->centroid.y(), it->centroid.z());
                double distanceB = distance(b->last_x, b->last_y, b->last_z, it->centroid.x(), it->centroid.y(), it->centroid.z());

                return distanceA < distanceB;
            });




        }

        if (!idk.empty()) {
            std::cout << "UPDATE!" << errorBoxes.size() << std::endl;
            db->updateBox((*idk.begin())->getId(), it->centroid.x(), it->centroid.y(), it->centroid.z(), it->dimensions.x(), it->dimensions.y(), it->dimensions.z());
            matchedCluster->push_back(*it);
            idk.erase(idk.begin());
            it = errorClusters->erase(it);

            if (!idk.empty()) {
                errorBoxes.insert(errorBoxes.end(), idk.begin(), idk.end());
            }
        }
        else{
            ++it;
        }

}
}

void TaskManager::handleOtherErrors(bool error1, bool error2) {
    if(error1 || error2)
    {

        emit updateStatus(QString("ERROR  :  CHECK TRAY FIX AND RE RUN UPDATE"));
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


bool TaskManager::isClusterAlreadyInList(int clusterId) {
    if (matchedCluster->empty()) {
        return false;
    }
    else
    {
        for (const auto& cluster : *matchedCluster) {
            if (cluster.clusterId == clusterId) {
                return true;
            }
        }
    }
    return false;
}
bool TaskManager::dimensionsMatch(const ClusterInfo &cluster, const Box &box1) {
    // Define a threshold for matching dimensions
    //std::cout << "DIMENSION MATHCINGr" << std::endl;
    float threshold = 1.5; // Adjust as needed
    float thresholdZ = 1.0;
    //auto conversion = 5.64634146;

    //Check if the dimensions are zero
    if (cluster.dimensions.x() == 0 || cluster.dimensions.y() == 0 || cluster.dimensions.z() == 0) {
        return false;
    }
    else if (box1.width == 0 || box1.height == 0 || box1.length == 0) {
        return false;
    }

    // Advanved check for the dimensions to assign the correct conversion factor
    if( cluster.dimensions.x() < cluster.dimensions.z() || cluster.dimensions.y() < cluster.dimensions.z()){
        //Case when box is sideways

        //Check for the box to have similar dimensions (x and y) to around 10 points
        if(std::abs(cluster.dimensions.x() - cluster.dimensions.y()) <= 10){
            // The dimensions x and y are similar within a tolerance of 10 points
            if (cluster.dimensions.x() + 30 < cluster.dimensions.z()){
                conversionY = 8.0f;
                conversionX = 12.85f;
            }
            else{
                conversionX = 8.0f;
                conversionY = 12.85f;
            }
            conversionZ = 8.72f;
        }
        else{
            if(cluster.clusterSize<5000){
                conversionX = 7.3f;
                conversionY = 10.0f;
                conversionZ = 10.0f;
            }
            else if(cluster.clusterSize<10000){
                conversionX = 7.2f;
                conversionY = 8.6f;
                conversionZ = 9.6f;
            }
            else{
                conversionX = 7.0f;
                conversionY = 8.0f;
                conversionZ = 9.0f;
            }
        }

    }
    else{
        //Case when box is upright

        //Check for the box to have similar dimensions (x and y) to around 10 points
        if (std::abs(cluster.dimensions.x() - cluster.dimensions.y()) <= 10) {
            // The dimensions x and y are similar within a tolerance of 10 points
            if(cluster.clusterSize < 7000){
                conversionX = 7.9f;
                conversionY = 7.9f;
                conversionZ = 10.0f;
            }
            else if(cluster.clusterSize<10000){
                conversionX = 7.45f;
                conversionY = 7.5f;
                conversionZ = 9.2f;
            }
            else{
                conversionX = 6.75f;
                conversionY = 6.75f;
                conversionZ = 9.99f;
            }
            std::cout << "Box with similar x and y dimensions and id of cluster " << cluster.clusterId << "." << std::endl;

        }
        else{
            if (cluster.clusterSize < 3000){
                conversionX = 5.5f;
                conversionY = 6.87;
                conversionZ = 8.2f;
            }
            else if(cluster.clusterSize<5000){
                conversionX = 6.21f;
                conversionY = 6.8f;
                conversionZ = 10.0f;
            }
            else if(cluster.clusterSize<10000){
                conversionX = 6.25f;
                conversionY = 7.5f;
                conversionZ = 9.99f;
            }
            else if(cluster.clusterSize<13000){
                conversionX = 6.0f;
                conversionY = 6.6f;
                conversionZ = 9.99f;
            }
            else if(cluster.clusterSize<15000){
                conversionX = 6.4f;
                conversionY = 6.70f;
                conversionZ = 9.99f;
            }
            else if(cluster.clusterSize<18000){
                conversionX = 8.1f;
                conversionY = 6.26f;
                conversionZ = 8.8f;
            }
            else{
                conversionX = 6.2f;
                conversionY = 6.6f;
                conversionZ = 8.8f;
            }

        }
    }

    std::cout << "Converted Dimensions: " << cluster.dimensions.x()/conversionX << " " << cluster.dimensions.y()/conversionY << " " << cluster.dimensions.z()/conversionZ << endl;
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
        heightMatch = std::abs(cluster.dimensions.z()/(conversionZ) - std::get<2>(dim1)) < thresholdZ;
        lengthMatch = std::abs(cluster.dimensions.y()/(conversionY) - std::get<1>(dim1)) < threshold;
        if(widthMatch && lengthMatch && heightMatch)
        {
            //Print converted Cluster dimensions
            std::cout << "Conversion Factors: " << conversionX << " " << conversionY << " " << conversionZ << endl;
            std::cout << "Converted Dimensions: " << cluster.dimensions.x()/conversionX << " " << cluster.dimensions.y()/conversionY << " " << cluster.dimensions.z()/conversionZ << endl;
            std::cout << "Cluster ID: " << cluster.clusterId << " Cluster Size: " << cluster.clusterSize << endl;
            //locaiton of the cluster
            std::cout << "Cluster Location: " << cluster.centroid.x() << " " << cluster.centroid.y() << " " << cluster.centroid.z() << std::endl;
            std::cout << "DImension pair " << std::get<0>(dim1) << " : " << std::get<1>(dim1) << " : "  << std::get<2>(dim1) <<  std::endl;
            //std::cout << "Dimensions of cluster: " << cluster.dimensions.x()/conversionX << " " << cluster.dimensions.y()/conversionY << " " << cluster.dimensions.z()/(conversionZ) << endl;
            match = true;
            break;
        }
    }



    return match;
}
