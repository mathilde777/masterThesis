#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "database.h"
#include <QObject>
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
    int executeTasks();
    int addBox();
    int findBox();
    void update();
    void prepTasks(int id);
    void getTasks(int trayId);
    void prepFirstFind();
    std::vector<std::unique_ptr<Task>> queue;
    std::shared_ptr<Database> db; // Use shared_ptr for Database
   // std::shared_ptr<PCL_3D> pcl;

    //PLY Files
    std::string filePathBoxes = "/home/suleyman/Desktop/MasterThesis/ModelsV2/2box_new_Color_PointCloud.ply";
    std::string filePathEmpty = "/home/suleyman/Desktop/MasterThesis/ModelsV2/emptry_tray_Color_PointCloud.ply";
    std::string filePathBrownBox = "/home/suleyman/Desktop/MasterThesis/ModelsV2/brownBox_new_Color_PointCloud.ply";

    //Database db;

 signals:
     void taskCompleted();
     void refresh();

 public slots:
    int onTaskCompleted();



};

#endif // TASKMANAGER_H
