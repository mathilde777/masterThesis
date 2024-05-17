
#ifndef ADDTASK_H
#define ADDTASK_H

#include "taskType.h"

class AddTask : public TaskType {
public:
    explicit AddTask(std::shared_ptr<Task> task);

    void executeTask() override;

    // Other specific functions for AddTask
    void specificFunctionForAddTask();
};

#endif // ADDTASK_H

