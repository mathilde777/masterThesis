#ifndef FINDTASK_H
#define FINDTASK_H

#include <memory>
#include "BaseTask.h"
#include "Database.h"
#include "Eigen/src/Core/Matrix.h"
#include "Task.h"
#include <Eigen/Core>
#include <vector>
#include "Detection2D.h"
#include "Detection3D.h"
#include "Result.h"
#include "PhotoProcessing.h"


class FindTask{
public:
   FindTask(std::shared_ptr<Database> db, std::vector<std::shared_ptr<KnownBox>> knownBoxes);
    void execute(const std::shared_ptr<Task>& task) ;

private:
    void executeFullTrayScan();
    void executePartialTrayScan( const Eigen::Vector3f& lastPosition);
    void processBoxDetectionResult(  std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    void handleSuccessfulBoxFound(  Eigen::Vector3f& result);
    void handleFailedBoxDetection();
    Eigen::Vector3f matchBox( std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);

    void removeExecutedTask();
    std::shared_ptr<Task> task;
      bool noResults;
    std::shared_ptr<Database> db;
      std::shared_ptr<PhotoProcessor> photoProcessing;
 std::vector<std::shared_ptr<KnownBox>> knownBoxes;
    bool checkFlaggedBoxes(int productId);

};

#endif // FINDTASK_H
