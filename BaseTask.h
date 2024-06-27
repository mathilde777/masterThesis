#ifndef BASETASK_H
#define BASETASK_H

#include <memory>
#include "Database.h"

class BaseTask {
public:
    virtual ~BaseTask() = default;
    virtual void execute() = 0;
};

#endif // BASETASK_H

