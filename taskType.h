#ifndef TASKTYPE_H
#define TASKTYPE_H



#include <memory>

class Task; // Forward declaration of Task class

class TaskType {
public:
    explicit TaskType(std::shared_ptr<Task> task) : task(task) {}
    virtual ~TaskType() = default;

    virtual void executeTask() = 0; // Pure virtual function

protected:
    std::shared_ptr<Task> task;
};


#endif // TASKTYPE_H
