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

class FindTask : public BaseTask {
public:
    explicit FindTask(const std::shared_ptr<Task>& task);
    void execute() ;

private:
    void executeFullTrayScan();
    void executePartialTrayScan( const Eigen::Vector3f& lastPosition);
    void processBoxDetectionResult( const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    void handleSuccessfulBoxFound( const Eigen::Vector3f& result);
    void handleFailedBoxDetection();
    Eigen::Vector3f matchBox(const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);


    std::shared_ptr<Task> task;
      bool noResults;
    std::shared_ptr<Database> db;
    std::shared_ptr<PhotoProcessing> photoProcessing;

};

#endif // FINDTASK_H
