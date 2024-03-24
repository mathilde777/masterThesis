#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "database.h"
#include "qobject.h"
#include "task.h"

class TaskManager : public QObject {
    Q_OBJECT

public:
    TaskManager();
    ~TaskManager();
    void addTask(int boxId, int trayId, int task);
    void trayDocked(int id);
    int executeTasks();
    int onTaskCompleted();
    int addBox();
    int findBox();
    void update();
    void getTasks(int trayId);
     std::vector<Task> queue;
    Database db;


};

#endif // TASKMANAGER_H
