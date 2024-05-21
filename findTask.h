#ifndef FINDTASK_H
#define FINDTASK_H

#include "taskType.h"

class FindTask : public TaskType {
public:
    explicit FindTask();

    void executeTask(std::shared_ptr<Task> task) override;
    void specificFunctionForFindTask();
private:
    int trained;
};

#endif // FINDTASK_H

