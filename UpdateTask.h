#ifndef UPDATETASK_H
#define UPDATETASK_H

#include <memory>
#include "BaseTask.h"
#include "Database.h"
#include "TaskFunctions.h"
#include "Box.h"
#include "clusters.h"
#include <vector>
#include <future>
#include "Detection2D.h"
#include "Detection3D.h"
#include "Result.h"

class UpdateTask : public BaseTask {
public:
    explicit UpdateTask(int trayId);
    void execute(std::shared_ptr<Database> db) override;

private:
    std::shared_ptr<Database> db;
    int trayId;
    std::vector<std::shared_ptr<Box>> trayBoxes;
    std::shared_ptr<std::vector<ClusterInfo>> matchedCluster;
    std::shared_ptr<std::vector<ClusterInfo>> errorClusters;
    std::vector<std::shared_ptr<Box>> errorBoxes;



    std::shared_ptr<PhotoProcessing> photoProcessing;

    bool checkForExtraBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                            const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    bool checkForMissingBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                              const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> matchClustersToBoxes(
        const std::vector<std::shared_ptr<Box>>& trayBoxes,
        const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> matchBoxesToClusters(
        const std::vector<std::shared_ptr<Box>>& trayBoxes,
        const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    void matchBoxes( const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes);
    void handleErrorBoxes();
    void handleOtherErrors(bool error1, bool error2);
    void sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes);
    bool isClusterAlreadyInList(int clusterId);
    void putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes);
};

#endif // UPDATETASK_H
