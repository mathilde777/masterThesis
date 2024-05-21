#ifndef TASKTYPE_H
#define TASKTYPE_H



#include "qobject.h"
#include "qtmetamacros.h"
#include <memory>
#include "task.h"

class TaskType: public QObject {
    Q_OBJECT
public: // For
    explicit TaskType();
    virtual ~TaskType() = default;

    virtual void executeTask(std::shared_ptr<Task> task) = 0; // Pure virtual function

protected:
    std::shared_ptr<Task> task;
};


#endif // TASKTYPE_H
