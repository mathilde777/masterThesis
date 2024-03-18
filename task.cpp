
#include "task.h"

Task::Task(int id, int task_type,int boxId,int trayId):
    id(id),task_type(task_type), boxId(boxId),trayId(trayId){}

// Getters
int Task::getId() const {
    return id;
}

int Task::getType() const {
    return task_type;
}

int Task::getTray() const {
    return trayId;
}

int Task::getBoxId() const {
    return boxId;
}

// Setters
void Task::setId(int taskId) {
    id = taskId;
}

void Task::setType(int taskType) {
    task_type = taskType;
}

void Task::setTray(int taskTray) {
    trayId = taskTray;
}

void Task::setBoxId(int taskBoxId) {
    boxId = taskBoxId;
}



