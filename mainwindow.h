#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QTimer>
#include <QLabel>
#include <QSlider>
#include <memory>
#include "TaskManager.h"
#include "database.h"
#include "task.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void findButtonClicked();
    void addButtonClicked();
    void trayButtonClicked(int trayNumber);

private:
    void populateBoxLists();

    std::shared_ptr<Database> db;
    std::unique_ptr<TaskManager> tm;
    std::unique_ptr<Ui::MainWindow> ui;
    std::vector<int> knownBoxes;
    std::vector<int> notStored;
    std::vector<int> storedBoxes;
    QList<int> addList;
    QList<int> findList;
    QList<int> trayList;

    QLineEdit *idLineEdit = nullptr;
    QLineEdit *trayIdLineEdit = nullptr;
    QPushButton *findButton = nullptr;
    QPushButton *addButton = nullptr;
    QPushButton *trayButtons[3];
    QPushButton *trayLeaving = nullptr;
    int dockedTray = 0;
    QComboBox *findComboBox = nullptr;
    QComboBox *boxComboBox = nullptr;
    QComboBox *trayComboBox = nullptr;
    QLabel *selectBoxLabel = nullptr;
    QLabel *selectBoxLabel2 = nullptr;
    QLabel *selectTrayLabel = nullptr;
    QVBoxLayout *findLayout = nullptr;
    QVBoxLayout *addLayout = nullptr;
    QHBoxLayout *toleranceLayout = nullptr;
    QSlider *toleranceSlider = nullptr;
    QTimer *trayTimer = nullptr;

signals:
    void trayDocked();
};



#endif // MAINWINDOW_H
