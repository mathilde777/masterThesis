#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <memory>
#include <vector>
#include <deque>
#include <unordered_map>
#include <Eigen/Core>
#include <QThread>
#include <QEventLoop>
#include <QObject>
#include "AddTask.h"
#include "Database.h"
#include "Detection2D.h"
#include "FindTask.h"
#include "TaskPreparer.h"
#include "Detection3D.h"
#include "Result.h"
#include "BaseTask.h"
#include "PhotoProcessing.h"
#include "UpdateTask.h"
#include "qtmetamacros.h"
#include "FindTask.h"
#include "UpdateTask.h"
#include "AddTask.h"
#include "TaskFunctions.h"

class TaskManager : public QObject {
    Q_OBJECT

public:
    explicit TaskManager(std::shared_ptr<Database> db);
    ~TaskManager();

    void prepTasks(int id);
    void trayDocked();
    void update(int id);
    void executeUpdateTask(int tray);
    void updateKnownBoxes();
    void calibrateTray(int position, double height);

private slots:
    void onTaskPrepared(std::shared_ptr<Task> task);
    void onTaskCompleted();
    void updateUiStatus(const QString& message);
    void preparingDone();

private:
    void startExecutionLoop();
    void waitForTasks();
    void executeTasks();
    void executeFindTask(const std::shared_ptr<Task>& task);
    void executeAddTask(const std::shared_ptr<Task>& task);
    void removeExecutedTask(const std::shared_ptr<Task>& task);
    bool checkFlaggedBoxes(int productId);

    std::vector<std::shared_ptr<KnownBox>> knownBoxes;
    std::deque<std::shared_ptr<Task>> executingQueue;
    bool taskExecuting = false;
    bool donePreparing = false;
    //bool noResults = false;
    int tray;
    std::shared_ptr<Database> db;
    std::vector<std::shared_ptr<Box>> trayBoxes;
    std::shared_ptr<UpdateTask> updateTask;
    std::shared_ptr<FindTask> findTask;
    std::shared_ptr<AddTask> addTask;
    Eigen::Vector3f refernce;

signals:
    void taskPrepared();
    void taskCompleted();
    void trayDockedUpdate();
    void updateStatus(const QString& status);
    void errorOccurredTask(const QString& error, int taskId);
    void refresh();
};

#endif // TASKMANAGER_H
