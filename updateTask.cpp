#include "updateTask.h"
#include <future>
#include <iostream>

#include "detection2D.h"
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


UpdateTask::UpdateTask(): TaskType() {}

void UpdateTask::executeTask(std::shared_ptr<Task> task) {
    // Implementation of update task
    std::cout << "Executing Update Task" << std::endl;
    trayBoxes = db->getAllBoxesInTray(task->trayId);
}

void UpdateTask::specificFunctionForUpdateTask() {
    // Implementation of a specific function for UpdateTask
}

bool UpdateTask::checkFlaggedBoxes(int productId)
{
    for(const auto& box: knownBoxes)
    {
        if(box->productId == productId)
        {
            std::cout << "box trained "<< box->trained << std::endl;
            if( box->trained == 1)
            {
                return true;
            }

            else
            {
                return false;

            }

        }
    }
    return false;
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

void UpdateTask::sortResultsByDistance(std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Box>& box) {
    std::sort(results->begin(), results->end(), [box](const ClusterInfo& a, const ClusterInfo& b) {
        return sortByDistance(a, b, box);
    });
}
bool UpdateTask::compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2) {
    return boxPtr1->id < boxPtr2->id;
}



void UpdateTask::sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    std::sort(trayBoxes.begin(), trayBoxes.end(), [this](const std::shared_ptr<Box>& a, const std::shared_ptr<Box>& b) {
        return this->compareBoxPtrByID(a, b);
    });

}

void UpdateTask::update(int id) {
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
bool UpdateTask::checkForExtraBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                                     const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    return trayBoxes.size() < resultsCluster->size();
}

bool UpdateTask::checkForMissingBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                                       const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    return trayBoxes.size() > resultsCluster->size();
}
std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> UpdateTask::matchClustersToBoxes(
    const std::vector<std::shared_ptr<Box>>& trayBoxes,
    const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> matches;
    std::cout << "CLUSTERS TO BOXES " <<    std::endl;
    for (const auto& cluster : *resultsCluster) {
        std::vector<std::shared_ptr<Box>> matchedBoxes;
        for (const auto& box : trayBoxes) {
            if (DimensionMatch::dimensionsMatch(cluster, *box)) {
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

std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> UpdateTask::matchBoxesToClusters(
    const std::vector<std::shared_ptr<Box>>& trayBoxes,
    const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> matches;
    std::cout << "BOX TO CLUSTERS " <<    std::endl;
    for (const auto& box : trayBoxes) {
        std::vector<ClusterInfo> matchedClusters;
        for (const auto& cluster : *resultsCluster) {
            if (DimensionMatch::dimensionsMatch(cluster, *box)) {
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
void UpdateTask::handleMatchedBoxes(const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes) {
    if (matchedBoxes.empty()) {
        std::cout << "ERROR: UNRECOGNIZABLE BOX!" << std::endl;
        errorBoxes.push_back(box);
        return;
    }
    else if (matchedBoxes.size() == 1) {
        std::cout << "UPDATING POSITION OF A BOX" << std::endl;
        auto it = matchedBoxes.begin();

        if (isClusterAlreadyInList(it->clusterId)) {
            errorBoxes.push_back(box);
            return;
        }

        auto directory = "/home/user/windows-share";
        auto PNGPath = photoProcessing->findLatestPngFile(directory);
        if (!PNGPath) {
            std::cout << "Error: No PNG file found" << std::endl;
            return; // Return early if no PNG file is found
        }
        std::cout << "Path: " << PNGPath->c_str() << std::endl;

        // Check box dimensions and crop accordingly
        if (it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()) {
            if (it->dimensions.x() > it->dimensions.y()) {
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            } else {
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            }
        } else {
            if (it->dimensions.x() > it->dimensions.y()) {
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            } else {
                photoProcessing->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            }
        }

        auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
        if (!PNGPathCropped) {
            std::cout << "Error: No cropped PNG file found" << std::endl;
            return; // Return early if no cropped PNG file is found
        }
        std::cout << "Cropped Path: " << PNGPathCropped->c_str() << std::endl;

        std::cout << "Looking for box with id " << box->getBoxId() << std::endl;

        // Check with 2D
        std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);
        if (!ret2 || ret2->empty()) {
            std::cout << "Error: run2D returned null or empty results" << std::endl;
            return; // Return early if run2D fails
        }

        if (checkFlaggedBoxes(box->getBoxId())) {
            for (const auto& res : *ret2) {
                std::cout << "2D Box: " << res.label << std::endl;
            }

            if (ret2->front().label != box->getBoxId()) {
                db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(),
                              matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
                matchedCluster->push_back(matchedBoxes[0]);
                return; // Return early if the label does not match
            } else {
                db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(),
                              matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
                matchedCluster->push_back(matchedBoxes[0]);
                return;
            }
        } else {
            // Save image or perform other operations
            db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(),
                          matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
            matchedCluster->push_back(matchedBoxes[0]);
            photoProcessing->storeCroppedImage(PNGPathCropped->c_str(),box->getBoxId());
            return;
        }
    }

    else

    {
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
            else

            {
                std::cout << "NOT TRAINED - SAVE IMAGE" << std::endl;
                photoProcessing->storeCroppedImage(PNGPathCropped->c_str(),box->getBoxId());
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


}
void UpdateTask::handleErrorBoxes() {
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

    if(errorClusters->size() == 1 && errorBoxes.size() == 1)
    {
        std::cout << "UPDATE!" << errorBoxes.size() << std::endl;
        db->updateBox((*errorBoxes.begin())->getId(), errorClusters->begin()->centroid.x(), errorClusters->begin()->centroid.y(), errorClusters->begin()->centroid.z(), errorClusters->begin()->dimensions.x(), errorClusters->begin()->dimensions.y(), errorClusters->begin()->dimensions.z());
        matchedCluster->push_back(*it);


        //Get latest png from PhotoProcessing
        auto directory = "/home/user/windows-share";
        auto PNGPath = photoProcessing->findLatestPngFile(directory);
        std::cout << "Path: " << PNGPath->c_str() << std::endl;

        //check if Path is correct, return string if not correct ==> Error
        if (!PNGPath) {
            std::cout << "Error: No PNG file found" << std::endl;

        }
        //Integrate Cropping
        //Box is sided
        if(  errorClusters->begin()->dimensions.x() <  errorClusters->begin()->dimensions.z() ||  errorClusters->begin()->dimensions.y() <  errorClusters->begin()->dimensions.z()){
            photoProcessing->cropToBox(PNGPath->c_str(),  errorClusters->begin()->centroid.x(),  errorClusters->begin()->centroid.y(),  errorClusters->begin()->dimensions.y(),  errorClusters->begin()->dimensions.x());
        }
        else{
            photoProcessing->cropToBox(PNGPath->c_str(),  errorClusters->begin()->centroid.x(),  errorClusters->begin()->centroid.y(),  errorClusters->begin()->dimensions.x(),  errorClusters->begin()->dimensions.y());
        }

        auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
        std::cout << "Crooped Path: " << PNGPathCropped->c_str() << std::endl;

        //check if Path is correct, return string if not correct ==> Error
        if (!PNGPathCropped) {
            std::cout << "Error: No PNG file found" << std::endl;
            ;
        }
        else {
            //std::cout << "Looking for box with id " << std::endl;
        }

        std::cout << "NOT TRAINED - SAVE IMAGE" << std::endl;
        photoProcessing->storeCroppedImage(PNGPathCropped->c_str(),(*errorBoxes.begin())->getBoxId());


    }


}

void UpdateTask::handleOtherErrors(bool error1, bool error2) {
    if(error1 || error2)
    {

        emit updateStatus(QString("ERROR  :  CHECK TRAY FIX AND RE RUN UPDATE"));
    }
}
void UpdateTask::deleteClusterById(std::shared_ptr<std::vector<ClusterInfo>> resultsCluster, int id) {
    auto& clusters = *resultsCluster;

    auto it = std::remove_if(clusters.begin(), clusters.end(), [id](const ClusterInfo &cluster) {
        return cluster.clusterId == id;
    });
    clusters.erase(it, clusters.end());
}

void UpdateTask::putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    auto partitionIter = std::partition(trayBoxes.begin(), trayBoxes.end(), [](const std::shared_ptr<Box>& boxPtr) {
        const Box& box = *boxPtr;
        return box.last_x != 0 || box.last_y != 0 || box.last_z != 0;
    });
}


bool UpdateTask::isClusterAlreadyInList(int clusterId) {
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

/**
bool UpdateTask::dimensionsMatch(const ClusterInfo &cluster, const Box &box1) {
    // Define a threshold for matching dimensions
    //std::cout << "DIMENSION MATHCINGr" << std::endl;
    float threshold = 2.5; // Adjust as needed
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
            if(cluster.clusterSize<7000){
                conversionX = 6.3f;
                conversionY = 6.5f;
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
                conversionX = 6.6f;
                conversionY = 6.9f;
                conversionZ = 9.0f;
            }
            else if(cluster.clusterSize<10000){
                conversionX = 7.82f;
                conversionY = 7.85f;
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
                conversionX = 7.65f;
                conversionY = 6.87;
                conversionZ = 8.2f;
            }
            else if(cluster.clusterSize<5000){
                conversionX = 7.0f;
                conversionY = 6.7f;
                conversionZ = 10.0f;
            }
            else if(cluster.clusterSize<9000){
                conversionX = 7.25f;
                conversionY = 6.5f;
                conversionZ = 8.99f;
            }
            else if(cluster.clusterSize<10000){
                conversionX = 5.62f;
                conversionY = 5.62f;
                conversionZ = 10.6f;
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
**/
