#include "pcl_3d.h"
#include "task.h"
#include "result.h"
#include <memory.h>
#include "3D_detection.h"
#include <cmath> // For std::abs
//std::shared_ptr<std::vector<ClusterInfo>>
std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f lastPosititon,Eigen::Vector3f dimensions) {
    PCL_3D pcl3d;
    auto conversionX = 6.50f;
    auto conversionY = 8.00f;
    auto conversionZ = 9.99f;
    // Example file paths and vectors for reference
    std::shared_ptr<PhotoProcessor> photoProcessing = std::make_shared<PhotoProcessor>();
    //Get latest png from PhotoProcessing
    auto directory = "/home/user/windows-share";
    auto boxFilePath = photoProcessing->findLatestPlyFile(directory);
    std::cout << "Path: " << boxFilePath->c_str() << std::endl;

    //check if Path is correct , return string if not correct ==> Error
    if(!boxFilePath)
    {
        std::cout << "Error: No PLY file found" << std::endl;
    }
    else{
        std::cout << "PLY file found" << std::endl;
    }



    //std::string trayFilePath = "/home/suleyman/Desktop/MasterThesis/ModelsV4/empty.ply";

    std::string trayFilePath = "/home/user/windows-share/empty/2024.5.14.9.17.50_Color_PointCloud.ply";

    //auto refPoint = pcl3d.calibrateTray(boxFilePath->c_str(), 690);
    auto refPoint = Eigen::Vector3f(456, 363.967, 699.949);
    //Update dimension by conversion factor (multiply by conversion factor for x,y) and z by conversion factor*1.5
    //dimensions = Eigen::Vector3f(dimensions.x()*conversionX,dimensions.y()*conversionY,dimensions.z()*(conversionZ));

    auto boundingBoxInfo = pcl3d.findBoundingBox(boxFilePath->c_str(), trayFilePath,refPoint,lastPosititon,dimensions);

    for (auto loc : boundingBoxInfo)
    {
        cout << "Cluster ID: " << loc.clusterId << endl;
        cout << "Centroid: " << loc.centroid.x() << " " << loc.centroid.y() << " " << loc.centroid.z() << endl;
        cout << "Dimensions: " << loc.dimensions.x()<< " " << loc.dimensions.y() << " " << loc.dimensions.z() << endl;
        cout << "Cluster Size Algo: " << loc.clusterSize << endl;
        std::cout << "Converted Dimensions: " << loc.dimensions.x()/conversionX << " " << loc.dimensions.y()/conversionY << " " << loc.dimensions.z()/(conversionZ) << endl;
        cout << "Orientation: " << loc.orientation.x() << " " << loc.orientation.y() << " " << loc.orientation.z() << " " << loc.orientation.w() << endl;
    }
    return std::make_shared<std::vector<ClusterInfo>>(boundingBoxInfo);

}

std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( ) {
    PCL_3D pcl3d;
    auto conversionX = 6.50f;
    auto conversionY = 8.00f;
    auto conversionZ = 9.99f;
    std::shared_ptr<PhotoProcessor> photoProcessing = std::make_shared<PhotoProcessor>();

    // Example file paths and vectors for reference
    auto directory = "/home/user/windows-share";
    auto boxFilePath = photoProcessing->findLatestPlyFile(directory);
    std::cout << "Path: " << boxFilePath->c_str() << std::endl;

    //check if Path is correct , return string if not correct ==> Error
    if(!boxFilePath)
    {
        std::cout << "Error: No PLY file found" << std::endl;
    }
    else{
        std::cout << "PLY file found" << std::endl;
    }
    //std::string trayFilePath = "/home/suleyman/Desktop/MasterThesis/ModelsV4/empty.ply";
    std::string trayFilePath = "/home/user/windows-share/empty/2024.5.14.9.17.50_Color_PointCloud.ply";

    //auto refPoint = pcl3d.calibrateTray(boxFilePath->c_str(), 690);

    auto refPoint = Eigen::Vector3f(458.649, 359, 699.949);
    auto boundingBoxInfo = pcl3d.findBoundingBox(boxFilePath->c_str(), trayFilePath,refPoint);
    for (auto loc : boundingBoxInfo)
    {
        cout << "Cluster ID: " << loc.clusterId << endl;
        cout << "Centroid: " << loc.centroid.x() << " " << loc.centroid.y() << " " << loc.centroid.z() << endl;
        cout << "Dimensions: " << loc.dimensions.x()<< " " << loc.dimensions.y() << " " << loc.dimensions.z() << endl;
        cout << "Cluster Size Algo: " << loc.clusterSize << endl;
        //std::cout << "Converted Dimensions: " << loc.dimensions.x()/conversionX << " " << loc.dimensions.y()/conversionY << " " << loc.dimensions.z()/(conversionZ) << endl;
        cout << "Orientation: " << loc.orientation.x() << " " << loc.orientation.y() << " " << loc.orientation.z() << " " << loc.orientation.w() << endl;
    }
    return std::make_shared<std::vector<ClusterInfo>>(boundingBoxInfo);

}
/**
std::tuple<double,double,double> getConversions(const ClusterInfo& cluster)
{
    float threshold = 1.5; // Adjust as needed
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
        if(std::abs(cluster.dimensions.x() - cluster.dimensions.y()) <= 20){
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
        if (std::abs(cluster.dimensions.x() - cluster.dimensions.y()) <= 20) {
            // The dimensions x and y are similar within a tolerance of 10 points
            if(cluster.clusterSize<10000){
                conversionX = 7.1f;
                conversionY = 7.1f;
                conversionZ = 10.9f;
            }
            else{
                conversionX = 6.75f;
                conversionY = 6.75f;
                conversionZ = 9.99f;
            }

        }
        else{
            if(cluster.clusterSize<5000){
                conversionX = 4.21f;
                conversionY = 4.8f;
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
            else{
                conversionX = 8.1f;
                conversionY = 6.26f;
                conversionZ = 8.8f;
            }

        }
    }


}

/**
std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> matchClusterWithBox(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::shared_ptr<Box>& box) {
    std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> matches = std::make_shared<std::vector<std::pair<ClusterInfo, double>>>();
    std::cout << "Matchingggg " << std::endl;
    double tolerance = 2.0; // Adjust as needed using the slider
    auto conversion = 5.64634146;
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
            std::cout << "DImension pair " << std::get<0>(dimensions) << " : " << std::get<1>(dimensions) << " : "  << std::get<2>(dimensions) <<  std::endl;
                std::cout << "Dimensions of cluster: " << cluster.dimensions.x()/conversion << " " << cluster.dimensions.y()/conversion << " " << cluster.dimensions.z()/(conversion*1.5) << endl;
            double widthDiff = std::abs(std::get<0>(dimensions) - cluster.dimensions.x()/conversion);
                double heightDiff = std::abs(std::get<1>(dimensions) - cluster.dimensions.y()/conversion);
                double lengthDiff = std::abs(std::get<2>(dimensions) - cluster.dimensions.z()/ (conversion*1.5));


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
        std::cout << "Match found: Cluster ID " << std::endl; //<< matches->front().first.clusterId << " matches with Box ID " << box->getId() << std::endl;
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
    double tolerance = 2;
    auto conversion = 5.64634146;
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
                double widthDiff = std::abs(std::get<0>(dimensions) - cluster.dimensions.x()/conversion);
                double heightDiff = std::abs(std::get<1>(dimensions) - cluster.dimensions.y()/conversion);
                double lengthDiff = std::abs(std::get<2>(dimensions) - cluster.dimensions.z()/ (conversion*1.5));

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
**/
