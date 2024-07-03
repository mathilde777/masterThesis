#ifndef _DETECTION3D_H
#define _DETECTION3D_H

#include "Box.h"
#include "Eigen/src/Core/Matrix.h"
#include "clusters.h"
#include <memory>
#include "PhotoProcessing.h"


std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f  reference, Eigen::Vector3f lastPosititon,  Eigen::Vector3f dimensions);
std::shared_ptr<std::vector<ClusterInfo>> run3DDetection( Eigen::Vector3f  reference);
Eigen::Vector3f calibrate(double height);


#endif
