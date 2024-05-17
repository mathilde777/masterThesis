#ifndef FINDTASK_H
#define FINDTASK_H

#include "taskType.h"

class FindTask : public TaskType {
public:
    explicit FindTask(std::shared_ptr<Task> task);

    void executeTask() override;
    void specificFunctionForFindTask();
private:
    int trained;
};

#endif // FINDTASK_H

