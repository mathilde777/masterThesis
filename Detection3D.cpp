#include "pcl_3d.h"
#include "Task.h"
#include "Result.h"
#include <memory.h>
#include "Detection3D.h"
#include <cmath> // For std::abs

std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f  reference,Eigen::Vector3f lastPosititon,Eigen::Vector3f dimensions) {
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



    //--------- EDIT
    //
    //the the file path must be editited to your specific laptop
    std::string trayFilePath = "/home/user/windows-share/empty/2024.5.16.13.38.37_Color_PointCloud.ply";
    //
    //---------

    auto boundingBoxInfo = pcl3d.findBoundingBox(boxFilePath->c_str(), trayFilePath,reference,lastPosititon,dimensions);

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

std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f  reference) {
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


    //--------- EDIT
    //
    //the the file path must be editited to your specific laptop
    std::string trayFilePath = "/home/user/windows-share/empty/2024.5.16.13.38.37_Color_PointCloud.ply";
    //
    //---------


    auto boundingBoxInfo = pcl3d.findBoundingBox(boxFilePath->c_str(), trayFilePath,reference);
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

Eigen::Vector3f calibrate(double height)
{
    PCL_3D pcl3d;

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
    auto refPoint = pcl3d.calibrateTray(boxFilePath->c_str(), height);
    return refPoint;
}
