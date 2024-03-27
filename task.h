#ifndef TASK_H
#define TASK_H


// 0 -> find
// 1-> add
//2 -> update
class Task{
public:
    int id;
    int task_type;
    int boxId;
    int trayId;

    Task(int id, int task_type,int boxId,int trayId);


    // Getters
    int getId() const;
    int getType() const;
    int getTray() const;
    int getBoxId() const;

    // Setters
    void setId(int taskId);
    void setType(int taskType);
    void setTray(int taskTray);
    void setBoxId(int taskBoxId);
};



#endif // TASK_H
