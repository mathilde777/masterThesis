#include "pcl_3d.h"
#include "task.h"
#include "result.h"
#include <memory.h>
#include "3D_detection.h"
#include <cmath> // For std::abs
//std::shared_ptr<std::vector<ClusterInfo>>
int run3DDetection() {
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


    return 1;

}




void matchClusterWithBox(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, std::shared_ptr<Box>& boxes) {
    std::vector<ClusterInfo> matches;
    for (const auto& cluster : *clusters) {
        for (const auto& box : boxes) {
            // Check if the dimensions are approximately equal with some tolerance
            double tolerance = 0.4; // Adjust as needed using trhe slider
            if (std::abs(box->width - cluster.dimensions.x()) < tolerance &&
                std::abs(box->height - cluster.dimensions.y()) < tolerance &&
                std::abs(box->length - cluster.dimensions.z()) < tolerance) {
                // Match found

                matches.push_back(cluster);

            }
        }
    }
        if(matches.size() == 1)
        {
            std::cout << "Match found: Cluster ID " << matches.front().clusterId << " matches with Box ID " << boxes->getId() < std::endl;
        }
        else if( matches.size() > 1)

        {

        }
}



