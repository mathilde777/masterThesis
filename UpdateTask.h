#ifndef UPDATETASK_H
#define UPDATETASK_H

#include <memory>
#include "Database.h"
#include "Box.h"
#include "clusters.h"
#include <vector>
#include <future>
#include "Detection2D.h"
#include "Detection3D.h"
#include "Result.h"
#include "TaskFunctions.h"

class UpdateTask:public QObject {
    Q_OBJECT

public:
     UpdateTask(std::shared_ptr<Database> db);
    void execute(int trayId,  Eigen::Vector3f refernce) ;

private:
    std::shared_ptr<Database> db;
    int trayId;
    std::vector<std::shared_ptr<Box>> trayBoxes;
    std::shared_ptr<std::vector<ClusterInfo>> matchedCluster;
    std::shared_ptr<std::vector<ClusterInfo>> errorClusters;
    std::vector<std::shared_ptr<Box>> errorBoxes;
      Eigen::Vector3f refernce;


    std::shared_ptr<PhotoProcessor> photoProcessing;

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
    void handleMatchedBoxes(std::shared_ptr<Database> db, const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes);
    void handleErrorBoxes(std::shared_ptr<Database> db);


signals:
    void taskCompleted();
    void updateStatus(const QString &status);
};

#endif // UPDATETASK_H
