#ifndef TASKFUNCTIONS_H
#define TASKFUNCTIONS_H

#include <memory>
#include <vector>
#include <Eigen/Core>
#include "Box.h"
#include "clusters.h"
#include "Task.h"
#include "knownBox.h"

class TaskFunctions {
public:
    static bool checkFlaggedBoxes(int productId, const std::vector<std::shared_ptr<KnownBox>>& knownBoxes);

    static Eigen::Vector3f matchBox(const std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Task>& task, bool& noResults);
    static bool dimensionsMatch(const ClusterInfo& cluster, const Box& box);
    static void sortResultsByDistance(std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Box>& box);
    static bool compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2);
};

#endif // TASKFUNCTIONS_H
