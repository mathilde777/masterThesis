#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "database.h"
#include <QObject>
#include "qtimer.h"
#include "task.h"
#include <memory> // For std::shared_ptr
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


    //PLY Files
    std::string filePathBoxes = "/home/suleyman/Desktop/MasterThesis/ModelsV2/2box_new_Color_PointCloud.ply";
    std::string filePathEmpty = "/home/suleyman/Desktop/MasterThesis/ModelsV2/emptry_tray_Color_PointCloud.ply";
    std::string filePathBrownBox = "/home/suleyman/Desktop/MasterThesis/ModelsV2/brownBox_new_Color_PointCloud.ply";

    //Database db;
    int currentTaskIndex;
    bool preparingNextTask;
    int taskToExecuteIndex;
    void prepTask(int index);
     QTimer* executionTimer;

    std::vector<std::shared_ptr<Task>> executingQueue;
    std::vector<std::shared_ptr<Task>> preparedQueue;


    bool donePreparing;
    int run3DDetectionThread();
    std::vector<std::shared_ptr<Box>> trayBoxes;
     std::vector<std::shared_ptr<Box>> possibleSameSize;

    void findBoxesOfSameSize(const Box& box1);
    void update(int trayId);
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
