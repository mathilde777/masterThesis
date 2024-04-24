#include "pcl_3d.h"
#include "task.h"
#include "result.h"
#include <memory.h>
#include "3D_detection.h"
#include <cmath> // For std::abs
//std::shared_ptr<std::vector<ClusterInfo>>
std::shared_ptr<std::vector<ClusterInfo>> run3DDetection() {
    PCL_3D pcl3d;

    // Example file paths and vectors for reference
    std::string boxFilePath = "path/to/box.ply";
    std::string trayFilePath = "path/to/tray.ply";
    Eigen::Vector3f referencePoint(0.0f, 0.0f, 0.0f); // Example reference point
    Eigen::Vector3f prevLocation(0.0f, 0.0f, 0.0f);   // Example previous location
    float height = 10.0f; // Example height

    // Calibrate tray
    Eigen::Vector3f calibratedPoint = pcl3d.calibrateTray(trayFilePath, height);

    // Transform point cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>()); // Example point cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr transformedCloud = pcl3d.transformToReferencePoint(cloud, calibratedPoint);

    // Find bounding box
    std::shared_ptr<std::vector<ClusterInfo>> boundingBoxInfo = std::make_shared<std::vector<ClusterInfo>>(pcl3d.findBoundingBox(boxFilePath, trayFilePath, referencePoint, prevLocation));

    return boundingBoxInfo;

}




std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> matchClusterWithBox(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::shared_ptr<Box>& box) {
    std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> matches;
    double tolerance = 0.4; // Adjust as needed using trhe slider
    std::vector<std::tuple<double, double, double>> dimensionPairs = {
        {box->width, box->height, box->length},
        {box->width, box->length, box->height},
        {box->height, box->width, box->length},
        {box->height, box->length, box->width},
        {box->length, box->width, box->height},
        {box->length, box->height, box->width}
    };
    for (const auto& cluster : *clusters) { 
            for (const auto& dimensions : dimensionPairs) {
                double widthDiff = std::abs(std::get<0>(dimensions) - cluster.dimensions.x());
                double heightDiff = std::abs(std::get<1>(dimensions) - cluster.dimensions.y());
                double lengthDiff = std::abs(std::get<2>(dimensions) - cluster.dimensions.z());


                if (widthDiff < tolerance && heightDiff < tolerance && lengthDiff < tolerance) {
                double distance = std::sqrt(std::pow(box->last_x - cluster.centroid.x(), 2) +
                                            std::pow(box->last_y - cluster.centroid.y(), 2) +
                                            std::pow(box->last_z - cluster.centroid.z(), 2));
                matches->emplace_back(cluster, distance);
            }
    }
    }

        if(matches->size() == 1)
        {
        std::cout << "Match found: Cluster ID " << matches->front().first.clusterId << " matches with Box ID " << box->getId() << std::endl;
        }
        else if( matches->size() > 1)

        {
        std::sort(matches->begin(), matches->end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
        }
        else if(matches->size() <= 0 )
        {
           std::cout << "No Match found: Cluster ID " << std::endl;
        }


        return matches;
}



void updateBoxLocations(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::vector<std::shared_ptr<Box>>& boxes) {
    double tolerance = 0.4;
    for (const auto& cluster : *clusters) {
        std::shared_ptr<Box> bestMatch;
        double bestDifference = std::numeric_limits<double>::max();

        for (const auto& box : boxes) {
            // Check all combinations of box dimensions against cluster dimensions
            std::vector<std::tuple<double, double, double>> dimensionPairs = {
                {box->width, box->height, box->length},
                {box->width, box->length, box->height},
                {box->height, box->width, box->length},
                {box->height, box->length, box->width},
                {box->length, box->width, box->height},
                {box->length, box->height, box->width}
            };

            for (const auto& dimensions : dimensionPairs) {
                double widthDiff = std::abs(std::get<0>(dimensions) - cluster.dimensions.x());
                double heightDiff = std::abs(std::get<1>(dimensions) - cluster.dimensions.y());
                double lengthDiff = std::abs(std::get<2>(dimensions) - cluster.dimensions.z());

                if (widthDiff < tolerance && heightDiff < tolerance && lengthDiff < tolerance) {
                    double totalDifference = widthDiff + heightDiff + lengthDiff;

                    // Update the best match if the total difference is smaller
                    if (totalDifference < bestDifference) {
                        bestDifference = totalDifference;
                        bestMatch = box;
                    }
                }
            }
        }

        if (bestMatch) {
            bestMatch->last_x = cluster.centroid.x();
            bestMatch->last_y = cluster.centroid.y();
            bestMatch->last_z = cluster.centroid.z();
            std::cout << "Box ID " << bestMatch->id << " updated location to (" << bestMatch->last_x << ", " << bestMatch->last_y << ", " << bestMatch->last_z << ")" << std::endl;
        } else {
            std::cout << "Error: No matching box found for Cluster ID " << cluster.clusterId << std::endl;
        }
    }
}



