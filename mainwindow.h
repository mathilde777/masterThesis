#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "database.h"
#include "task.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    std::vector<Task> queue;
    Database db;
private slots:
    void findButtonClicked();
    void addButtonClicked();
    void trayButtonClicked(int trayNumber);
    void executeTasks();
    void onTaskCompleted();

private:
    QLineEdit *idLineEdit;
    QLineEdit *trayIdLineEdit;
    QPushButton *findButton;
    QPushButton *addButton;
    QPushButton *trayButtons[3];
    QPushButton *trayLeaving;
    Ui::MainWindow *ui;
    int dockedTray;

signals:
    void taskCompleted();
};
#endif // MAINWINDOW_H

