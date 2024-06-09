#include "UpdateTask.h"
#include <iostream>
#include "Detection3D.h"
#include "clusters.h"

UpdateTask::UpdateTask(int trayId) : trayId(trayId), matchedCluster(std::make_shared<std::vector<ClusterInfo>>()), errorClusters(std::make_shared<std::vector<ClusterInfo>>()) {}

void UpdateTask::execute(std::shared_ptr<Database> db) {
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
        auto PNGPath = PhotoProcessing::getInstance()->findLatestPngFile(directory);
        if (!PNGPath) {
            std::cout << "Error: No PNG file found" << std::endl;
            return;
        }
        std::cout << "Path: " << PNGPath->c_str() << std::endl;

        if (it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()) {
            if (it->dimensions.x() > it->dimensions.y()) {
                PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            } else {
                PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            }
        } else {
            if (it->dimensions.x() > it->dimensions.y()) {
                PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
            } else {
                PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
            }
        }

        auto PNGPathCropped = PhotoProcessing::getInstance()->findLatestCroppedImage();
        if (!PNGPathCropped) {
            std::cout << "Error: No cropped PNG file found" << std::endl;
            return;
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
            PhotoProcessing::getInstance()->storeCroppedImage(PNGPathCropped->c_str(), box->getBoxId());
            return;
        }
    } else {
        auto it = std::remove_if(matchedBoxes.begin(), matchedBoxes.end(), [&](const ClusterInfo& cluster) {
            return isClusterAlreadyInList(cluster.clusterId);
        });
        matchedBoxes.erase(it, matchedBoxes.end());

        TaskFunctions::sortResultsByDistance(matchedCluster, box);

        for (auto it = matchedBoxes.begin(); it != matchedBoxes.end(); ) {
            auto directory = "/home/user/windows-share";
            auto PNGPath = PhotoProcessing::getInstance()->findLatestPngFile(directory);
            if (!PNGPath) {
                std::cout << "Error: No PNG file found" << std::endl;
                break;
            }

            if (it->dimensions.x() < it->dimensions.z() || it->dimensions.y() < it->dimensions.z()) {
                if (it->dimensions.x() > it->dimensions.y()) {
                    PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
                } else {
                    PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
                }
            } else {
                if (it->dimensions.x() > it->dimensions.y()) {
                    PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.x(), it->dimensions.y());
                } else {
                    PhotoProcessing::getInstance()->cropToBox(PNGPath->c_str(), it->centroid.x(), it->centroid.y(), it->dimensions.y(), it->dimensions.x());
                }
            }

            auto PNGPathCropped = PhotoProcessing::getInstance()->findLatestCroppedImage();
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
                PhotoProcessing::getInstance()->storeCroppedImage(PNGPathCropped->c_str(), box->getBoxId());
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
    // Logic to handle error boxes
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
