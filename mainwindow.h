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
#include <QTextEdit>
#include "knownBox.h"
#include <QFormLayout>

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
   void populateBoxLists();
    void updateDockedInfo();
   void updateButtonClicked();
    void newBoxClicked();
    void addNewKnownBox(const QString &width, const QString &height, const QString &length, const QString &name);
   void calibrate();
    void addImages();
public slots:
   void updateStatusText(const QString& message);
   //void handleErrorTask(QString errorMessage, int taskId);
    //void handleErrorUpdate(QString errorMessage);
private:


    std::shared_ptr<Database> db;
    std::unique_ptr<TaskManager> tm;
    std::unique_ptr<Ui::MainWindow> ui;
    std::vector<std::shared_ptr<KnownBox>> knownBoxes;
    std::vector<std::shared_ptr<KnownBox>> notStored;
    std::vector<std::pair<int, std::string>> storedBoxes;
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
    QLabel *dockedInfoLabel;
    QTextEdit *statusText;
     QPushButton *updateButton = nullptr;
     QPushButton *addNewBox = nullptr;
     QPushButton *calibration = nullptr;

     void createInputDialog();

     QPushButton *addTrainingImages = nullptr;
     void processTrainingImage(const QString& selectedBox, const QString& fileName);

signals:
    void trayDocked();
    void refresh();
};



#endif // MAINWINDOW_H
