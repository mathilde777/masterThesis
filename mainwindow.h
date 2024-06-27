#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QDateTime>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTimer>
#include <QMessageBox>
#include <memory>
#include "Database.h"
#include "TaskManager.h"

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
    void updateDockedInfo();
    void updateStatusText(const QString& message);
    void updateButtonClicked();
    void newBoxClicked();
    void addImages();
    void trayButtonClicked();
    void calibrate();
private:
    Ui::MainWindow *ui;
    int dockedTray;
    std::shared_ptr<Database> db;
    std::unique_ptr<TaskManager> tm;
    QLabel *dockedInfoLabel;
    QTextEdit *statusText;
    QComboBox *findComboBox;
    QComboBox *boxComboBox;
    QComboBox *trayComboBox;
    QComboBox *runTrayTasksComboBox;
    QPushButton *findButton;
    QPushButton *addButton;
    QPushButton *runTasksButton;
    QPushButton *updateButton;
    QPushButton *calibration;
    QPushButton *addNewBox;
    QPushButton *addTrainingImages;
    QHBoxLayout *toleranceLayout;
    QTimer *trayTimer;
    std::vector<std::pair<int, std::string>> storedBoxes;
    std::vector<std::shared_ptr<KnownBox>> notStored;
    QLabel *selectBoxLabel;
    QLabel *selectBoxLabel2;
    QLabel *selectTrayLabel;

    void populateBoxLists();
    void createInputDialog();
    void addNewKnownBox(const QString &width, const QString &height, const QString &length, const QString &name);
};

#endif // MAINWINDOW_H
