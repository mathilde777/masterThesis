#ifndef UPDATETASK_H
#define UPDATETASK_H

#include "box.h"
#include "clusters.h"
#include "taskType.h"
#include "database.h"
#include "photoProcessing.h"
#include "detection2D.h"
#include "dimensionMatching.h"

class UpdateTask : public TaskType {
     Q_OBJECT

public:
    explicit UpdateTask();
    void executeTask(std::shared_ptr<Task> task) override;
    void specificFunctionForUpdateTask();
    void deleteClusterById(std::shared_ptr<std::vector<ClusterInfo>> resultsCluster, int id);
    void putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes);
    bool isClusterAlreadyInList(int clusterId);
    bool dimensionsMatch(const ClusterInfo& cluster, const Box& box1);
    void handleErrorBoxes();
    void handleOtherErrors(bool error1, bool error2);
    bool checkFlaggedBoxes(int productId);
    void update(int id);
    std::shared_ptr<PhotoProcessor> photoProcessing;


bool  checkForExtraBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                            const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);


bool checkForMissingBoxes(const std::vector<std::shared_ptr<Box>>& trayBoxes,
                                 const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);

std::vector<std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>> matchClustersToBoxes(    const std::vector<std::shared_ptr<Box>>& trayBoxes,
                                                                                            const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
std::vector<std::pair<std::shared_ptr<Box>, std::vector<ClusterInfo>>> matchBoxesToClusters(
    const std::vector<std::shared_ptr<Box>>& trayBoxes,
    const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);

private:
    int trayId;
    //float conversionX = 0.0f;
   // float conversionY = 0.0f;
    //float conversionZ = 0.0f;
    std::shared_ptr<Database> db =std::make_shared<Database>();
    std::shared_ptr<std::vector<ClusterInfo>> matchedCluster =  std::make_shared<std::vector<ClusterInfo>>();
    std::shared_ptr<std::vector<ClusterInfo>> errorClusters =  std::make_shared<std::vector<ClusterInfo>>();
    std::vector<std::shared_ptr<KnownBox>> knownBoxes;
    std::vector<std::shared_ptr<Box>> trayBoxes;
    void sortResultsByDistance(std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Box>& box);
    bool compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2);
    void sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes);
    std::vector<std::shared_ptr<Box>> errorBoxes;
    void handleMatchedBoxes(const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes);


signals:
    void updateStatus(const QString& message);


};

#endif // UPDATETASK_H
