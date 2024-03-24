#ifndef DATABASE_H
#define DATABASE_H

#include "task.h"
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <vector>
#include <QObject>
#include <mysql_connection.h>

using namespace sql;

class Database : public QObject {
    Q_OBJECT

private:
    sql::Connection* con;
    sql::mysql::MySQL_Driver*  driver;

public:
    Database();
    ~Database();
    void initializeDatabase();
    void closeDatabase();

    void addTask(int box_id, int task_type, int tray_id);
    std::vector<Task> getTasks(int tray_id);

    bool checkStoredBoxes(int boxId);
    bool checkKnownBoxes(int boxId);

    void getBoxInfo(int boxId);
    bool checkExistingBoxes(int tray_id, int box_id);
    void removeTaskFromQueue(int taskId);
    int getTrayId(int box_id);
    std::vector<int> getUnstoredBoxes();
    std::vector<int> getStoredBoxes();
    std::vector<int> getKnownBoxes();

public slots:
    void storeBox(int id, int tray);
    void removeStoredBox(int boxId);

signals:
    void taskCompleted();
};

#endif // DATABASE_H

