#ifndef TASKPREPARER_H
#define TASKPREPARER_H
#include <QObject>
#include <QThread>
#include <memory>
#include "database.h"
#include "task.h"

class TaskPreparer : public QObject {
    Q_OBJECT
public:
    explicit TaskPreparer(int trayId,std::shared_ptr<Database> db);
    ~TaskPreparer();

public slots:
    void prepareTask();

signals:
    void taskPrepared(std::shared_ptr<Task> task);
    void finished();

private:
    std::vector<std::shared_ptr<Task>> tasks;
    int trayId;
    void getTasks();
    std::shared_ptr<Database> db;
};
#endif // TASKPREPARER_H
