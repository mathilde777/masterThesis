#include "FindTask.h"
#include <iostream>
#include "PhotoProcessing.h"
#include "TaskFunctions.h"

FindTask::FindTask( std::shared_ptr<Database> db, std::vector<std::shared_ptr<KnownBox>> knownBoxes) : knownBoxes(knownBoxes){}

void FindTask::execute(const std::shared_ptr<Task>& task) {
    this->task = task;
    noResults = false;
    std::cout << "finding box" << std::endl;
    Eigen::Vector3f lastPosition(task->getBox()->last_x, task->getBox()->last_y, task->getBox()->last_z);
    if (lastPosition == Eigen::Vector3f(0.0f, 0.0f, 0.0f)) {
        executeFullTrayScan();
    } else {
        executePartialTrayScan(lastPosition);
    }
}

void FindTask::executeFullTrayScan() {
    std::cout << "FULL TRAY SCAN " << std::endl;
    auto resultsCluster = run3DDetection();
    processBoxDetectionResult(resultsCluster);
}

void FindTask::executePartialTrayScan(const Eigen::Vector3f& lastPosition) {
    std::cout << "PARTIAL TRAY SCAN " << std::endl;
    auto resultsCluster = run3DDetection(lastPosition, task->getBox()->getClusterDimensions());
    processBoxDetectionResult(resultsCluster);
}


void FindTask::processBoxDetectionResult( std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster) {
    auto result = matchBox(resultsCluster);
    std::cout << "Result: " << result << std::endl;

    if (result != Eigen::Vector3f(0.0f, 0.0f, 0.0f)) {
        handleSuccessfulBoxFound(result);
    } else {
        if(noResults)
        {
            handleFailedBoxDetection();
            removeExecutedTask();

        }
        else
        {
            executeFullTrayScan();
        }
    }
}

void FindTask::removeExecutedTask()
    {
        //emit a remove task
    }

void FindTask::handleSuccessfulBoxFound( Eigen::Vector3f& result) {
    std::cout << "task completed time to remove stored box: " << task->getBoxId() << std::endl;
    db->removeStoredBox(task->getBoxId());
    std::cout << "FOUND AT " << result << std::endl;
}

void FindTask::handleFailedBoxDetection() {
    std::cout << "BOX not found: " << task->getBoxId() << std::endl;
    std::cout << "Scanning the whole tray again" << std::endl;
    executeFullTrayScan();
}

Eigen::Vector3f  FindTask::matchBox( std::shared_ptr<std::vector<ClusterInfo>>& results)
{
    Eigen::Vector3f  result = Eigen::Vector3f(0.0f,0.0f,0.0f);

    // Null pointer check
    if (!results || results->empty()) {
        std::cerr << "ERROR: Invalid input data." << std::endl;
        return result;
    }

    // Filter results based on dimensions
    auto it = std::remove_if(results->begin(), results->end(), [&](const ClusterInfo& info) {
        return !TaskFunctions::dimensionsMatch(info, *task->getBox());
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
        auto PNGPath = photoProcessing->findLatestPngFile("/home/user/windows-share");
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
        if(TaskFunctions::checkFlaggedBoxes(task->getBox()->getBoxId(),knownBoxes))
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
            photoProcessing->storeCroppedImage(PNGPathCropped->c_str(),task->getBox()->getBoxId());
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
       // for (const auto& box_id : possibleSameSize) {
         //   std::cout << "Also these boxes are possible: " << box_id->getBoxId() << std::endl;
       // }
        auto box = task->getBox();
        TaskFunctions::sortResultsByDistance(results, task->getBox());

        for (auto itn = results->begin(); itn != results->end();) {

            auto PNGPath = photoProcessing->findLatestPngFile("/home/user/windows-share");
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
                photoProcessing->storeCroppedImage(PNGPathCropped->c_str(),task->getBox()->getBoxId());
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


bool FindTask::checkFlaggedBoxes(int productId)
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
