#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "TaskManager.h"
#include "database.h"
#include "qcombobox.h"
#include "task.h"
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Database db;
    TaskManager tm;
    std::vector<int> knownBoxes;
    std::vector<int> notStored;
    std::vector<int> storedBoxes;
    void populateBoxLists();

private slots:
    void findButtonClicked();
    void addButtonClicked();
    void trayButtonClicked(int trayNumber);


private:
    QLineEdit *idLineEdit;
    QLineEdit *trayIdLineEdit;
    QPushButton *findButton;
    QPushButton *addButton;
    QPushButton *trayButtons[3];
    QPushButton *trayLeaving;
    Ui::MainWindow *ui;
    int dockedTray;
    QList<int> addList;
    QList<int> findList;
    QList<int> trayList;
    QComboBox *findComboBox;
    QComboBox *boxComboBox;
    QComboBox *trayComboBox;
    QLabel *selectBoxLabel;
    QLabel *selectBoxLabel2;
    QLabel *selectTrayLabel;
    QVBoxLayout *findLayout;
    QVBoxLayout *addLayout;
    QHBoxLayout *toleranceLayout;
    QSlider *toleranceSlider;
    QTimer *trayTimer;


signals:
    void trayDocked();
};
#endif // MAINWINDOW_H

