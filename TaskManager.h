#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "clusters.h"
#include "database.h"
#include <QObject>
#include "detection2D.h"
#include "qtimer.h"
#include "task.h"
#include <memory> // For std::shared_ptr
#include "photoProcessing.h"
//#include "/home/suleyman/Desktop/MasterThesis/library/lib/include/pcl_3d.h"

class TaskManager : public QObject {
    Q_OBJECT

public:
    explicit TaskManager(std::shared_ptr<Database> db,std::shared_ptr<std::vector<std::shared_ptr<KnownBox>>> knownBoxes);
    ~TaskManager();

    std::vector<std::shared_ptr<Task>> queue;
    std::shared_ptr<Database> db;

    //Task functions
    void addTask(int boxId, int trayId, int task);

    void prepTasks(int id);
    void getTasks(int trayId);

    //Tray control
    void trayDocked();




    int currentTaskIndex;
    bool preparingNextTask;
    int taskToExecuteIndex;
    QTimer* executionTimer;

    float conversionX;
    float conversionY;
    float conversionZ;
    std::deque<std::shared_ptr<Task>> executingQueue;
    std::vector<std::shared_ptr<Task>> preparedQueue;
    std::shared_ptr<PhotoProcessor> photoProcessing;

    bool donePreparing;
    int run3DDetectionThread();
    std::vector<std::shared_ptr<Box>> trayBoxes;
    std::vector<std::shared_ptr<Box>> possibleSameSize;


    std::shared_ptr<std::vector<ClusterInfo>> matchedCluster =  std::make_shared<std::vector<ClusterInfo>>();
    std::shared_ptr<std::vector<ClusterInfo>> errorClusters =  std::make_shared<std::vector<ClusterInfo>>();


signals:
    void trayDockedUpdate();
    void taskPrepared(); //this indicates that a task is prepared and to add it to the queue
    void taskCompleted();
    void refresh();
    void taskExecutionCompleted();
    void errorOccurredTask(QString errorMessage , int taskId);
    void errorOccurredUpdate(QString errorMessage);
    void updateStatus(const QString& message);

private slots:
    void executeTasks();
    void waitForTasks();
    void startExecutionLoop();
    void preparingDone();
    void onTaskCompleted();
    void onTaskPrepared(std::shared_ptr<Task> task);

private:


    void update(int trayId);
    bool dimensionsMatch(const ClusterInfo& cluster, const Box& box);
    int tray;

    Eigen::Vector3f match_box(std::shared_ptr<std::vector<ClusterInfo>> results, std::shared_ptr<Task> task);
    Eigen::Vector3f handleNoResults(std::shared_ptr<Task> task);
    bool noResults;

    void deleteClusterById(std::shared_ptr<std::vector<ClusterInfo>> resultsCluster, int id);
    void putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes);
    bool isClusterAlreadyInList(int clusterId);
    bool compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2);
    void sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes);
    void sortResultsByDistance(std::shared_ptr<std::vector<ClusterInfo>>& results, const std::shared_ptr<Box>& box);

    std::vector<std::shared_ptr<Box>> errorBoxes;
    bool taskExecuting;
    void executeAddBoxTask(const std::shared_ptr<Task>& task);
    void executeFindBoxTask(const std::shared_ptr<Task>& task);
    void executeFullTrayScan(const std::shared_ptr<Task>& task);
    void executePartialTrayScan(const std::shared_ptr<Task>& task, const Eigen::Vector3f& lastPosition);
    void processBoxDetectionResult(const std::shared_ptr<Task>& task, const std::shared_ptr<std::vector<ClusterInfo>>& resultsCluster);
    void handleSuccessfulBoxFound(const std::shared_ptr<Task>& task, const Eigen::Vector3f& result);
    void handleFailedBoxDetection(const std::shared_ptr<Task>& task);
    void removeExecutedTask(const std::shared_ptr<Task>& task);


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
    void handleErrorBoxes();
    void handleOtherErrors(bool error1, bool error2);
    void handleMatchedBoxes(const std::shared_ptr<Box>& box, std::vector<ClusterInfo>& matchedBoxes);
    void updateBoxInfo(std::shared_ptr<Box> box, const ClusterInfo& cluster);
    std::shared_ptr<std::vector<std::shared_ptr<KnownBox>>> knownBoxes;
    int checkFlaggedBoxes(int productId);
};

#endif // TASKMANAGER_H
