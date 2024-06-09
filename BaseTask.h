#ifndef BASETASK_H
#define BASETASK_H

#include <memory>
#include "Database.h"

class BaseTask {
public:
    virtual ~BaseTask() = default;
    virtual void execute(std::shared_ptr<Database> db) = 0;
};

#endif // BASETASK_H

