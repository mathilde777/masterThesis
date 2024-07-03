#ifndef _DETECTION3D_H
#define _DETECTION3D_H

#include "Box.h"
#include "clusters.h"
#include <memory>
#include "PhotoProcessing.h"


std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f  reference, Eigen::Vector3f lastPosititon,  Eigen::Vector3f dimensions);
std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f  reference);
//std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> matchClusterWithBox(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::shared_ptr<Box>& box);
//void updateBoxLocations(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::vector<std::shared_ptr<Box>>& boxes);
Eigen::Vector3f calibrate(double height);


#endif
