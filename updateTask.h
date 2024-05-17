#ifndef UPDATETASK_H
#define UPDATETASK_H

#include "taskType.h"

class UpdateTask : public TaskType {
public:
    explicit UpdateTask(std::shared_ptr<Task> task);
    void executeTask() override;

    // Other specific functions for UpdateTask
    void specificFunctionForUpdateTask();

private:
    int trayId;
};

#endif // UPDATETASK_H
