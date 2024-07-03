#ifndef FINDTASK_H
#define FINDTASK_H

#include <memory>

#include "Database.h"
#include "Eigen/src/Core/Matrix.h"
#include "Task.h"
#include <Eigen/Core>
#include <vector>
#include "PhotoProcessing.h"
#include "clusters.h"


class FindTask: public QObject {
    Q_OBJECT

public:
    FindTask(std::shared_ptr<Database> db, std::vector<std::shared_ptr<KnownBox>> knownBoxes);
    void execute(const std::shared_ptr<Task>& task,   Eigen::Vector3f refernce) ;

private:
    void executeFullTrayScan();
    void executePartialTrayScan( const Eigen::Vector3f& lastPosition);
    void processBoxDetectionResult(  std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    void handleSuccessfulBoxFound(  Eigen::Vector3f& result);
    void handleFailedBoxDetection();
    Eigen::Vector3f matchBox( std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    void removeExecutedTask();
    bool checkFlaggedBoxes(int productId);

    std::shared_ptr<Task> task;    
    std::shared_ptr<Database> db;
    std::shared_ptr<PhotoProcessor> photoProcessing;
    std::vector<std::shared_ptr<KnownBox>> knownBoxes;

    Eigen::Vector3f refernce;
    bool noResults;


signals:
    void taskCompleted();
    void updateStatus(const QString &status);
};

#endif // FINDTASK_H
