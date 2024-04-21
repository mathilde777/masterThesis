#ifndef TASK_H
#define TASK_H


#include "box.h"
#include <memory>

// 0 -> find
// 1-> add
//2 -> update
class Task{
public:
    int id;
    int task_type;
    int boxId;
    int trayId;
    std::shared_ptr<Box> box;


    Task(int id, int task_type, int boxId, int trayId, std::shared_ptr<Box> box)
        : id(id), task_type(task_type), boxId(boxId), trayId(trayId), box(box) {}

    // Getters
    int getId() const { return id; }
    int getType() const { return task_type; }
    int getTray() const { return trayId; }
    int getBoxId() const { return boxId; }
    std::shared_ptr<Box> getBox() const { return box; }

    // Setters
    void setId(int taskId) { id = taskId; }
    void setType(int taskType) { task_type = taskType; }
    void setTray(int taskTray) { trayId = taskTray; }
    void setBoxId(int taskBoxId) { boxId = taskBoxId; }
    void setBox(std::shared_ptr<Box> b) { box = b; }
};




#endif // TASK_H
