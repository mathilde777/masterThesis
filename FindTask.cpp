#include "FindTask.h"
#include <iostream>
#include "Detection2D.h"
#include "Detection3D.h"
#include "PhotoProcessing.h"
#include "TaskFunctions.h"

FindTask::FindTask(std::shared_ptr<Database> db, std::vector<std::shared_ptr<KnownBox>> knownBoxes)
    : db(db), knownBoxes(knownBoxes), noResults(false) {}

void FindTask::execute(const std::shared_ptr<Task>& task,  Eigen::Vector3f ref) {
    this->task = task;
    noResults = false;
    refernce = ref;
    std::cout << "finding box" << std::endl;
      emit updateStatus(QString("FIND : start to find box with id %1").arg(task->getBoxId()));
    Eigen::Vector3f lastPosition(task->getBox()->last_x, task->getBox()->last_y, task->getBox()->last_z);
    //first check if it is possible to do a partial scan
    if (lastPosition == Eigen::Vector3f(0.0f, 0.0f, 0.0f)) {
        emit updateStatus(QString("FIND ERROR : box with id %1 has no previous location -> scanning full tray").arg(task->getBoxId()));
        executeFullTrayScan();
    } else {
        executePartialTrayScan(lastPosition);
    }
}

void FindTask::executeFullTrayScan() {
    std::cout << "FULL TRAY SCAN " << std::endl;
     emit updateStatus(QString("FIND : running full 3D for %1").arg(task->getBoxId()));
    auto resultsCluster = run3DDetection(refernce);
    processBoxDetectionResult(resultsCluster);
}

void FindTask::executePartialTrayScan(const Eigen::Vector3f& lastPosition) {
    std::cout << "PARTIAL TRAY SCAN " << std::endl;
    emit updateStatus(QString("FIND : running partial 3D for %1").arg(task->getBoxId()));
    auto resultsCluster = run3DDetection(refernce, lastPosition, task->getBox()->getClusterDimensions());
    processBoxDetectionResult(resultsCluster);
}

void FindTask::processBoxDetectionResult(std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    auto result = matchBox(resultsCluster);
    std::cout << "Result: " << result << std::endl;


    if (result != Eigen::Vector3f(0.0f, 0.0f, 0.0f)) {
        handleSuccessfulBoxFound(result);
    } else {
        if (noResults) {
            handleFailedBoxDetection();

        } else {
            executeFullTrayScan();
        }
    }
    emit taskCompleted();
}


void FindTask::handleSuccessfulBoxFound(Eigen::Vector3f& result) {
    std::cout << "task completed time to remove stored box: " << task->getBoxId() << std::endl;
    QString resultString = QString("%1, %2, %3").arg(result.x()).arg(result.y()).arg(result.z());
    emit updateStatus(QString("FIND Successful: box found at %1").arg(resultString));
    db->removeStoredBox(task->getBoxId());
    std::cout << "FOUND AT " << result << std::endl;
}

void FindTask::handleFailedBoxDetection() {
    std::cout << "BOX not found: " << task->getBoxId() << std::endl;
    std::cout << "Scanning the whole tray again" << std::endl;
    emit updateStatus(QString("FIND ERROR : box with id %1 not found").arg(task->getBoxId()));
    //here it has failed to determine the location, one can chose to then rpovide teh last known location if needed

}

Eigen::Vector3f FindTask::matchBox(std::shared_ptr<std::vector<ClusterInfo>>& results) {
    Eigen::Vector3f result = Eigen::Vector3f(0.0f, 0.0f, 0.0f);

    if (!results || results->empty()) {
        std::cerr << "ERROR: Invalid input data." << std::endl;
        return result;
    }

    auto it = std::remove_if(results->begin(), results->end(), [&](const ClusterInfo& info) {
        return !TaskFunctions::dimensionsMatch(info, *task->getBox());
    });
    results->erase(it, results->end());

    switch (results->size()) {
    case 0:
        result = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
        if(!noResults)
        {
            noResults = true;
        }
        break;

    case 1: {
        const auto& centroid = results->begin()->centroid;
        auto PNGPath = photoProcessing->findLatestPngFile("/home/user/windows-share");
        if (!PNGPath) {
            std::cerr << "ERROR: No PNG file found" << std::endl;
            break;
        }

        const auto& itn = results->begin();
        if (itn->dimensions.x() < itn->dimensions.z() || itn->dimensions.y() < itn->dimensions.z()) {
            photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(),
                                       std::max(itn->dimensions.x(), itn->dimensions.y()), std::min(itn->dimensions.x(), itn->dimensions.y()));
        } else {
            photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(),
                                       std::min(itn->dimensions.x(), itn->dimensions.y()), std::max(itn->dimensions.x(), itn->dimensions.y()));
        }

        auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
        if (!PNGPathCropped) {
            std::cerr << "ERROR: No cropped PNG file found" << std::endl;
            break;
        }

        std::cout << "Looking for box with type ID: " << task->getBox()->getBoxId() << std::endl;
        std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);
        if (TaskFunctions::checkFlaggedBoxes(task->getBox()->getBoxId(), knownBoxes)) {
            if (!res2D->empty() && res2D->front().label == task->getBox()->getBoxId()) {
                if (centroid.size() >= 3) {
                    result << centroid(0), centroid(1), centroid(2);
                } else {
                    std::cerr << "ERROR: Invalid centroid data." << std::endl;
                }
            }
        } else {
            std::cout << "NOT TRAINED BOX IN FIND: " << task->getBox()->getBoxId() << std::endl;
            photoProcessing->storeCroppedImage(PNGPathCropped->c_str(), task->getBox()->getBoxId());
            if (centroid.size() >= 3) {
                result << centroid(0), centroid(1), centroid(2);
            } else {
                std::cerr << "ERROR: Invalid centroid data." << std::endl;
            }
        }
        break;
    }

    default: {
        std::cout << "Looking for box with ID: " << task->getBoxId() << std::endl;
        auto box = task->getBox();
        TaskFunctions::sortResultsByDistance(results, task->getBox());

        for (auto itn = results->begin(); itn != results->end();) {
            auto PNGPath = photoProcessing->findLatestPngFile("/home/user/windows-share");
            if (!PNGPath) {
                std::cerr << "ERROR: No PNG file found" << std::endl;
                break;
            }

            if (itn->dimensions.x() < itn->dimensions.z() || itn->dimensions.y() < itn->dimensions.z()) {
                photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(),
                                           std::max(itn->dimensions.x(), itn->dimensions.y()), std::min(itn->dimensions.x(), itn->dimensions.y()));
            } else {
                photoProcessing->cropToBox(PNGPath->c_str(), itn->centroid.x(), itn->centroid.y(),
                                           std::min(itn->dimensions.x(), itn->dimensions.y()), std::max(itn->dimensions.x(), itn->dimensions.y()));
            }

            auto PNGPathCropped = photoProcessing->findLatestCroppedImage();
            if (!PNGPathCropped) {
                std::cerr << "ERROR: No cropped PNG file found" << std::endl;
                break;
            }

            std::cout << "Looking for box with ID: " << box->getBoxId() << std::endl;
            std::shared_ptr<std::vector<DetectionResult>> res2D = run2D(PNGPathCropped->c_str(), 1);

            if (checkFlaggedBoxes(task->getBox()->getBoxId())) {
                if (!res2D->empty() && res2D->front().label != task->getBox()->getBoxId()) {
                    std::cout << "BOX type: " << res2D->front().label << std::endl;
                    itn = results->erase(itn);
                } else {
                    ++itn;
                }
            } else {
                photoProcessing->storeCroppedImage(PNGPathCropped->c_str(), task->getBox()->getBoxId());
                break; // Assuming no need to continue in this case
            }
        }

        if (!results->empty()) {
            const auto& centroid = results->front().centroid;
            if (centroid.size() >= 3) {
                result << centroid(0), centroid(1), centroid(2);
                std::cout << result << std::endl;
            }
        } else {
            result = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
            noResults = !noResults;
        }
        break;
    }
    }
    return result;
}

bool FindTask::checkFlaggedBoxes(int productId) {
    for (const auto& box : knownBoxes) {
        if (box->productId == productId) {
            std::cout << "box trained " << box->trained << std::endl;
            return box->trained == 1;
        }
    }
    return false;
}
