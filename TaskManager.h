#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "clusters.h"
#include "database.h"
#include <QObject>
#include "qtimer.h"
#include "task.h"
#include <memory> // For std::shared_ptr
#include "photoProcessing.h"
//#include "/home/suleyman/Desktop/MasterThesis/library/lib/include/pcl_3d.h"

class TaskManager : public QObject {
    Q_OBJECT

public:
    explicit TaskManager(std::shared_ptr<Database> db);
    ~TaskManager();
    void addTask(int boxId, int trayId, int task);
    void trayDocked();
    void prepTasks(int id);
    void getTasks(int trayId);

    std::vector<std::shared_ptr<Task>> queue;
    std::shared_ptr<Database> db; // Use shared_ptr for Database

    // std::shared_ptr<PCL_3D> pcl;
    //Database db;
    int currentTaskIndex;
    bool preparingNextTask;
    int taskToExecuteIndex;
    void prepTask(int index);
    QTimer* executionTimer;

    std::vector<std::shared_ptr<Task>> executingQueue;
    std::vector<std::shared_ptr<Task>> preparedQueue;

    // PhotoProcessor photoProcessing;
    std::shared_ptr<PhotoProcessor> photoProcessing;


    bool donePreparing;
    int run3DDetectionThread();
    std::vector<std::shared_ptr<Box>> trayBoxes;
    std::vector<std::shared_ptr<Box>> possibleSameSize;

    void findBoxesOfSameSize(const Box& box1);
    void update(int trayId);
    bool dimensionsMatch(const ClusterInfo& cluster, const Box& box);
    int tray;

    Eigen::Vector3f match_box(std::shared_ptr<std::vector<ClusterInfo>> results, std::shared_ptr<Task> task);
    Eigen::Vector3f handleNoResults(std::shared_ptr<Task> task);
    bool noResults;

    double distance(double x1, double y1, double z1, double x2, double y2, double z2);
    bool compareClosestToClusterCenter(const std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>& a,
                                       const std::pair<ClusterInfo, std::vector<std::shared_ptr<Box>>>& b);

    void deleteClusterById(std::shared_ptr<std::vector<ClusterInfo>> resultsCluster, int id);
    void putZeroLocationBoxesAtBack(std::vector<std::shared_ptr<Box>>& trayBoxes);
    bool isClusterAlreadyInList(int clusterId, const std::shared_ptr<std::vector<ClusterInfo>>& clusters);
    bool compareBoxPtrByID(const std::shared_ptr<Box>& boxPtr1, const std::shared_ptr<Box>& boxPtr2);
    void sortTrayBoxesByID(std::vector<std::shared_ptr<Box>>& trayBoxes) ;
signals:
    void trayDockedUpdate();
    void taskPrepared();
    void taskCompleted();
    void refresh();
    void taskExecutionCompleted();


private slots:
    void executeTasks();
    void waitForTasks();
    void startExecutionLoop();
    void preparingDone();
    void onTaskCompleted();
    void onTaskPrepared(std::shared_ptr<Task> task);
private:
    bool taskExecuting;


};

#endif // TASKMANAGER_H
