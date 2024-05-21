#ifndef DIMENSIONMATCHING_H
#define DIMENSIONMATCHING_H



#include "clusters.h" // Assuming these are your custom types
#include "box.h"          // Include necessary headers

class DimensionMatch {
public:
    static bool dimensionsMatch(const ClusterInfo &cluster, const Box &box1);

};
#endif // DIMENSIONMATCHING_H
