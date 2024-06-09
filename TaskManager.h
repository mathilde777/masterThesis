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
#include "Database.h"
#include "Detection2D.h"
#include "TaskPreparer.h"
#include "Detection3D.h"
#include "Result.h"
#include "BaseTask.h"
#include "PhotoProcessing.h"

class TaskManager : public QObject {
    Q_OBJECT

public:
    TaskManager(std::shared_ptr<Database> db);
    ~TaskManager();

    void prepTasks(int id);
    void trayDocked();
    void update(int id);

private slots:
    void onTaskPrepared(std::shared_ptr<Task> task);
    void onTaskCompleted();
    void preparingDone();

private:
    void startExecutionLoop();
    void waitForTasks();
    void executeTasks();

    std::vector<std::shared_ptr<Box>> knownBoxes; // Changed to vector
    std::deque<std::shared_ptr<BaseTask>> executingQueue; // Changed to deque for efficient pop_front
    bool taskExecuting;
    bool donePreparing;
    bool noResults;
    int tray;
    std::shared_ptr<Database> db;
    std::shared_ptr<PhotoProcessor> photoProcessing;

signals:
    void taskPrepared();
    void taskCompleted();
    void trayDockedUpdate();
    void updateStatus(const QString &status);
    void errorOccurredTask(const QString &error, int taskId);
    void refresh();
};

#endif // TASKMANAGER_H
