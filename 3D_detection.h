#ifndef _3D_DETECTION_H
#define _3D_DETECTION_H

#include "box.h"
#include "clusters.h"
#include <memory>
#include "photoProcessing.h"

std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f lastPosititon,  Eigen::Vector3f dimensions);
std::shared_ptr<std::vector<ClusterInfo>> run3DDetection();
//std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> matchClusterWithBox(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::shared_ptr<Box>& box);
//void updateBoxLocations(const std::shared_ptr<std::vector<ClusterInfo>>& clusters, const std::vector<std::shared_ptr<Box>>& boxes);



#endif
