#ifndef ADDTASK_H
#define ADDTASK_H

#include <memory>
#include "BaseTask.h"
#include "Database.h"
#include "Task.h"

class AddTask : public BaseTask {
public:
    explicit AddTask(const std::shared_ptr<Task>& task);
    void execute(std::shared_ptr<Database> db) override;

private:
    std::shared_ptr<Task> task;
};

#endif // ADDTASK_H
