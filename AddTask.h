#ifndef ADDTASK_H
#define ADDTASK_H

#include <memory>
#include "BaseTask.h"
#include "Database.h"
#include "Task.h"

class AddTask  : public QObject {
    Q_OBJECT

public:
    AddTask(std::shared_ptr<Database> db);
    void execute(const std::shared_ptr<Task>& task );

private:
    std::shared_ptr<Task> task;
    std::shared_ptr<Database> db;


signals:
    void taskCompleted();
};

#endif // ADDTASK_H
