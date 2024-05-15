#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H
#include "database.h"
#include "qobject.h"
class ErrorManager : public QObject {
    Q_OBJECT

public:
    explicit ErrorManager(std::shared_ptr<Database> db);
    ~ErrorManager();

    std::shared_ptr<Database> db;


    void handleError1();
    void handleError2();
    void handleErrorNewBox();


signals:


private slots:


private:

};


#endif // ERRORMANAGER_H
