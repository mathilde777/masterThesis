#include "TaskFunctions.h"
#include "PhotoProcessing.h"
#include "Detection3D.h"
#include <iostream>
#include <algorithm>
#include <cmath>

bool TaskFunctions::checkFlaggedBoxes(int productId, const std::vector<std::shared_ptr<Box>>& knownBoxes) {
    for (const auto& box : knownBoxes) {
        if (box->productId == productId) {
            return box->trained == 1;
        }
    }
    return false;
}



bool TaskFunctions::dimensionsMatch(const ClusterInfo& cluster, const Box& box) {
    float threshold = 2.5; // Adjust as needed
    float thresholdZ = 1.0;

    if (cluster.dimensions.x() == 0 || cluster.dimensions.y() == 0 || cluster.dimensions.z() == 0) {
        return false;
    }
    if (box.width == 0 || box.height == 0 || box.length == 0) {
        return false;
    }

    float conversionX, conversionY, conversionZ;
    if (cluster.dimensions.x() < cluster.dimensions.z() || cluster.dimensions.y() < cluster.dimensions.z()) {
        if (std::abs(cluster.dimensions.x() - cluster.dimensions.y()) <= 10) {
            if (cluster.dimensions.x() + 30 < cluster.dimensions.z()) {
                conversionY = 8.0f;
                conversionX = 12.85f;
            } else {
                conversionX = 8.0f;
                conversionY = 12.85f;
            }
            conversionZ = 8.72f;
        } else {
            if (cluster.clusterSize < 7000) {
                conversionX = 6.3f;
                conversionY = 6.5f;
                conversionZ = 10.0f;
            } else if (cluster.clusterSize < 10000) {
                conversionX = 7.2f;
                conversionY = 8.6f;
                conversionZ = 9.6f;
            } else {
                conversionX = 7.0f;
                conversionY = 8.0f;
                conversionZ = 9.0f;
            }
        }
    } else {
        if (std::abs(cluster.dimensions.x() - cluster.dimensions.y()) <= 10) {
            if (cluster.clusterSize < 7000) {
                conversionX = 6.6f;
                conversionY = 6.9f;
                conversionZ = 9.0f;
            } else if (cluster.clusterSize < 10000) {
                conversionX = 7.82f;
                conversionY = 7.85f;
                conversionZ = 9.2f;
            } else {
                conversionX = 6.75f;
                conversionY = 6.75f;
                conversionZ = 9.99f;
            }
        } else {
            if (cluster.clusterSize < 3000) {
                conversionX = 7.65f;
                conversionY = 6.87f;
                conversionZ = 8.2f;
            } else if (cluster.clusterSize < 5000) {
                conversionX = 7.0f;
                conversionY = 6.7f;
                conversionZ = 10.0f;
            } else if (cluster.clusterSize < 9000) {
                conversionX = 7.25f;
                conversionY = 6.5f;
                conversionZ = 8.99f;
            } else if (cluster.clusterSize < 10000) {
                conversionX = 5.62f;
                conversionY = 5.62f;
                conversionZ = 10.6f;
            } else if (cluster.clusterSize < 13000) {
                conversionX = 6.0f;
                conversionY = 6.6f;
                conversionZ = 9.99f;
            } else if (cluster.clusterSize < 15000) {
                conversionX = 6.4f;
                conversionY = 6.70f;
                conversionZ = 9.99f;
            } else if (cluster.clusterSize < 18000) {
                conversionX = 8.1f;
                conversionY = 6.26f;
                conversionZ = 8.8f;
            } else {
                conversionX = 6.2f;
                conversionY = 6.6f;
                conversionZ = 8.8f;
            }
        }
    }

    std::vector<std::tuple<double, double, double>> dimensionPairs = {
        {box.width, box.height, box.length},
        {box.width, box.length, box.height},
        {box.height, box.width, box.length},
        {box.height, box.length, box.width},
        {box.length, box.width, box.height},
        {box.length, box.height, box.width}
    };

    for (const auto& dim1 : dimensionPairs) {
        bool widthMatch = std::abs(cluster.dimensions.x() / conversionX - std::get<0>(dim1)) < threshold;
        bool heightMatch = std::abs(cluster.dimensions.z() / conversionZ - std::get<2>(dim1)) < thresholdZ;
        bool lengthMatch = std::abs(cluster.dimensions.y() / conversionY - std::get<1>(dim1)) < threshold;

        if (widthMatch && heightMatch && lengthMatch) {
            return true;
        }
    }

    return false;
}

void TaskFunctions::sortResultsByDistance(std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Box>& box) {
    std::sort(results->begin(), results->end(), [box](const ClusterInfo& a, const ClusterInfo& b) {
        auto distance = [](double x1, double y1, double z1, double x2, double y2, double z2) {
            return std::sqrt((x2 - x1) * (x2 - x1) +
                             (y2 - y1) * (y2 - y1) +
                             (z2 - z1) * (z2 - z1));
        };
        double distanceA = distance(a.centroid.x(), a.centroid.y(), a.centroid.z(),
                                    box->last_x, box->last_y, box->last_z);
        double distanceB = distance(b.centroid.x(), b.centroid.y(), b.centroid.z(),
                                    box->last_x, box->last_y, box->last_z);
        return distanceA < distanceB;
    });
}

bool TaskFunctions::compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2) {
    return boxPtr1->id < boxPtr2->id;
}
