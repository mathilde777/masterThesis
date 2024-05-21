#include "dimensionMatching.h"
#include <iostream>



bool DimensionMatch::dimensionsMatch(const ClusterInfo &cluster, const Box &box1) {
    // Define a threshold for matching dimensions
    //std::cout << "DIMENSION MATHCINGr" << std::endl;
    float threshold = 2.5; // Adjust as needed
    float thresholdZ = 1.0;
    //auto conversion = 5.64634146;

    float conversionX = 0.0f;
    float conversionY = 0.0f;
    float conversionZ = 0.0f;

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

    std::cout << "Converted Dimensions: " << cluster.dimensions.x()/conversionX << " " << cluster.dimensions.y()/conversionY << " " << cluster.dimensions.z()/conversionZ << '\n';
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
            std::cout << "Conversion Factors: " << conversionX << " " << conversionY << " " << conversionZ << std::endl;
            std::cout << "Converted Dimensions: " << cluster.dimensions.x()/conversionX << " " << cluster.dimensions.y()/conversionY << " " << cluster.dimensions.z()/conversionZ << std::endl;
            std::cout << "Cluster ID: " << cluster.clusterId << " Cluster Size: " << cluster.clusterSize << std::endl;
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
