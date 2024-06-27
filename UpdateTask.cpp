#include "UpdateTask.h"
#include <iostream>
#include "Detection3D.h"
#include "clusters.h"

UpdateTask::UpdateTask(std::shared_ptr<Database> db) : matchedCluster(std::make_shared<std::vector<ClusterInfo>>()), errorClusters(std::make_shared<std::vector<ClusterInfo>>()) {}

void UpdateTask::execute(int trayId) {
    trayId = trayId;
    std::cout << "Running update" << std::endl;

    trayBoxes = db->getAllBoxesInTray(trayId);
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
    auto matches = matchClustersToBoxes(trayBoxes, resultsCluster);

    // Matching boxes to clusters
    auto matchesC = matchBoxesToClusters(trayBoxes, resultsCluster);

    // Handling matched clusters and updating box information
    for (auto& match : matchesC) {
        handleMatchedBoxes(db, match.first, match.second);
    }

    for (const auto& cluster : *resultsCluster) {
        if (!isClusterAlreadyInList(cluster.clusterId)) {
            errorClusters->push_back(cluster);
        }
    }

    // Handling error boxes
    if (!errorBoxes.empty()) {
        handleErrorBoxes(db);
    }

    // Handling other errors
    handleOtherErrors(error1, error2);

    std::cout << "UPDATE: done" << std::endl;
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
    for (const auto& cluster : *resultsCluster) {
        std::vector<std::shared_ptr<Box>> matchedBoxes;
        for (const auto& box : trayBoxes) {
            if (TaskFunctions::dimensionsMatch(cluster, *box)) {
                matchedBoxes.push_back(box);
            }
        }
        matches.emplace_back(cluster, matchedBoxes);
    }
    return matches;
}

std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> UpdateTask::matchBoxesToClusters(
    const std::vector<std::shared_ptr<Box>>& trayBoxes,
    const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> matches;
    for (const auto& box : trayBoxes) {
        std::vector<ClusterInfo> matchedClusters;
        for (const auto& cluster : *resultsCluster) {
            if (TaskFunctions::dimensionsMatch(cluster, *box)) {
                matchedClusters.push_back(cluster);
            }
        }
        matches.emplace_back(box, matchedClusters);
    }
    return matches;
}

void UpdateTask::handleMatchedBoxes(std::shared_ptr<Database> db, const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes) {
    if (matchedBoxes.empty()) {
        std::cout << "ERROR: UNRECOGNIZABLE BOX!" << std::endl;
        errorBoxes.push_back(box);
        return;
    } else if (matchedBoxes.size() == 1) {
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

        std::shared_ptr<std::vector<DetectionResult>> ret2 = run2D(PNGPathCropped->c_str(), 1);
        if (!ret2 || ret2->empty()) {
            std::cout << "Error: run2D returned null or empty results" << std::endl;
            return;
        }

        if (TaskFunctions::checkFlaggedBoxes(box->getBoxId(), db->getKnownBoxes())) {
            for (const auto& res : *ret2) {
                std::cout << "2D Box: " << res.label << std::endl;
            }

            if (ret2->front().label != box->getBoxId()) {
                db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(),
                              matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
                matchedCluster->push_back(matchedBoxes[0]);
                return;
            } else {
                db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(),
                              matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
                matchedCluster->push_back(matchedBoxes[0]);
                return;
            }
        } else {
            db->updateBox(box->getId(), matchedBoxes[0].centroid.x(), matchedBoxes[0].centroid.y(), matchedBoxes[0].centroid.z(),
                          matchedBoxes[0].dimensions.x(), matchedBoxes[0].dimensions.y(), matchedBoxes[0].dimensions.z());
            matchedCluster->push_back(matchedBoxes[0]);
            photoProcessing->storeCroppedImage(PNGPathCropped->c_str(),box->getBoxId());
            return;
        }
    } else {
        auto it = std::remove_if(matchedBoxes.begin(), matchedBoxes.end(), [&](const ClusterInfo& cluster) {
            return isClusterAlreadyInList(cluster.clusterId);
        });
        matchedBoxes.erase(it, matchedBoxes.end());

        TaskFunctions::sortResultsByDistance(matchedCluster, box);

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
                std::cout << "Error: No cropped PNG file found" << std::endl;
                break;
            }

            std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);

            if (TaskFunctions::checkFlaggedBoxes(box->getBoxId(), db->getKnownBoxes())) {
                if (!res2D->empty()) {
                    if (res2D->front().label != box->getBoxId()) {
                        it = matchedBoxes.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    break;
                }
            } else {
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

void UpdateTask::handleErrorBoxes(std::shared_ptr<Database> db) {
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
                auto distance = [](double x1, double y1, double z1, double x2, double y2, double z2) {
                    return std::sqrt((x2 - x1) * (x2 - x1) +
                                     (y2 - y1) * (y2 - y1) +
                                     (z2 - z1) * (z2 - z1));
                };
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
    if (error1 || error2) {
        std::cout << "ERROR: CHECK TRAY, FIX AND RE-RUN UPDATE" << std::endl;
    }
}

void UpdateTask::sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    std::sort(trayBoxes.begin(), trayBoxes.end(), TaskFunctions::compareBoxPtrByID);
}

bool UpdateTask::isClusterAlreadyInList(int clusterId) {
    if (matchedCluster->empty()) {
        return false;
    } else {
        for (const auto& cluster : *matchedCluster) {
            if (cluster.clusterId == clusterId) {
                return true;
            }
        }
    }
    return false;
}

void UpdateTask::putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes) {
    std::partition(trayBoxes.begin(), trayBoxes.end(), [](const std::shared_ptr<Box>& boxPtr) {
        const Box& box = *boxPtr;
        return box.last_x != 0 || box.last_y != 0 || box.last_z != 0;
    });
}
