#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "database.h"
#include <QObject>
#include "task.h"
#include <memory> // For std::shared_ptr

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
    std::vector<Task> queue;
    std::shared_ptr<Database> db; // Use shared_ptr for Database

    //Database db;

 signals:
     void taskCompleted();
     void refresh();

 public slots:
    int onTaskCompleted();



};

#endif // TASKMANAGER_H
