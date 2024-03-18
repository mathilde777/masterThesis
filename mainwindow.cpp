#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),dockedTray(0)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    idLineEdit = new QLineEdit(this);
     trayIdLineEdit = new QLineEdit(this);
    findButton = new QPushButton("Find", this);
    connect(findButton, &QPushButton::clicked, this, &MainWindow::findButtonClicked);

    addButton = new QPushButton("Add", this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addButtonClicked);
    trayLeaving = new QPushButton("Tray leaving", this);



    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(idLineEdit);
    inputLayout->addWidget(findButton);
    inputLayout->addWidget(addButton);
    inputLayout->addWidget(trayIdLineEdit);
    inputLayout->addWidget(trayLeaving);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(inputLayout);

    QHBoxLayout *trayLayout = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        trayButtons[i] = new QPushButton(QString("Tray %1").arg(i + 1), this);
        trayLayout->addWidget(trayButtons[i]);
        connect(trayButtons[i], &QPushButton::clicked, this, [=]() { trayButtonClicked(i + 1); });
    }
    mainLayout->addLayout(trayLayout);

    connect(&db, &Database::taskCompleted, this, &::MainWindow::onTaskCompleted);
    connect(this, &MainWindow::taskCompleted, this, &::MainWindow::onTaskCompleted);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::findButtonClicked()
{
    QString id = idLineEdit->text();
    // Perform action with the entered ID
    QMessageBox::information(this, "Find", QString("Find button clicked with ID: %1").arg(id));
   Database db = Database();

    //check if knwon and stored
    bool known = db.checkKnownBoxes(id.toInt());
    bool stored = db.checkStoredBoxes(id.toInt());


    int tray = db.getTrayId(id.toInt());
    // add to database queue
    if(known && stored)
    {

        db.addTask(id.toInt(),2,tray);
        if(tray == dockedTray && tray == 0)
        {
            Task task = Task(0, 1,id.toInt(),dockedTray);
            queue.push_back(task);
        }

    }



}

void MainWindow::addButtonClicked()
{
    QString id = idLineEdit->text();
    // Perform action with the entered ID
    QMessageBox::information(this, "Add", QString("Add box wiht wiht id %1").arg(id));
    Database db = Database();
    QString tray = trayIdLineEdit->text();
    //check if this box is known and already stored
    bool known = db.checkKnownBoxes(id.toInt());
    bool stored = db.checkStoredBoxes(id.toInt());
     std::cout << "arrived" << std::endl;
    // add to database queue
     if(tray.toInt() == dockedTray && tray.toInt() == 0)
     {
        Task task = Task(0, 1,id.toInt(),dockedTray);
        queue.push_back(task);
        db.addTask(id.toInt(),1,dockedTray);
     }
    if(known && !stored)
    {
        std::cout << "adding box" << std::endl;
        db.addTask(id.toInt(),1,tray.toInt());
    }
}

void MainWindow::trayButtonClicked(int trayNumber)
{
    std::cout << "tray docked" << std::endl;
    dockedTray = trayNumber;
    QMessageBox::information(this, "Tray", QString("Tray %1 docked").arg(trayNumber));
    Database db = Database();

    // grab all the tasks from the database quu
    queue = db.getTasks(trayNumber);
    for (const Task& task : queue) {
        std::cout << task.getId() << std::endl; // Assuming Task has a defined output operator
    }

    executeTasks();
    //while stille executing he taks int eh queue the tray will not change
}

void MainWindow::executeTasks()
{
    Database db;
    if (!queue.empty()) {
        Task task = queue.front();
        if(task.getType() == 1)
        {
            std::cout << "adding box" << std::endl;
            db.storeBox(task.getBoxId(),task.getTray());

        }
        else if(task.getType()==2)
        {
            if(db.checkExistingBoxes(task.getTray(), task.getBoxId()))
            {
                 std::cout << "RUN 2D imageing" << std::endl;
                 std::cout << "RUN 3D imaging" << std::endl;
            }
            else
            {
                  std::cout << "RUN 3D imaging" << std::endl;
            }
            emit taskCompleted();

            //handling all the
        }
    } else {
        std::cout << "All tasks executed." << std::endl;
    }
}

void MainWindow::onTaskCompleted() {
    // Remove the completed task from the queue
    db.removeTaskFromQueue(queue.begin()->getId());
    queue.erase(queue.begin());
    if (!queue.empty()) {

        executeTasks();
    }
    else
    {
        std::cout << "UPDATE DATABASE - ALL TASKS DONE" << std::endl;
    }
    // Start executing the next task


}
