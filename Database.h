#ifndef DATABASE_H
#define DATABASE_H

#include "Task.h"
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <vector>
#include <QObject>
#include "knownBox.h"

//change

using namespace sql;
using namespace std;

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
    std::vector<std::shared_ptr<Task>> getTasks(int tray_id);

    bool checkStoredBoxes(int boxId);
    bool checkKnownBoxes(int boxId);

    std::tuple<int, double, double, double, double, double, double,Eigen::Vector3f> getBoxInfo(int Id);
    bool checkExistingBoxes(int tray_id, int box_id);
    void removeTaskFromQueue(int taskId);
    int getTrayId(int box_id);
    std::vector<int> getUnstoredBoxes();
    std::vector<std::pair<int, std::string>>  getStoredBoxes();
    std::vector<std::shared_ptr<KnownBox>> getKnownBoxes();
    std::vector<std::shared_ptr<Box>> getAllBoxesInTray(int trayId) ;
    std::shared_ptr<Box> getBox(int tray, int boxId);
    std::tuple<double, double, double> getBoxDimensions(int boxId);
    void updateBox(int id,double last_x, double last_y,double last_z,double cx, double cy,double cz);
    void newKnownBox( std::string name, double width, double height,double  lenght);
    void addTrainingImage(int box_id,std::string pic );
    void  addReference(int posId, float x,float y, float z );
    Eigen::Vector3f getReferences(int position);

public slots:
    void storeBox(int id, int tray);
    void removeStoredBox(int boxId);

signals:
   // void taskCompleted();
};

#endif // DATABASE_H

