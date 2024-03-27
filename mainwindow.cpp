#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "detection2D.h"
#include <QComboBox>
#include <QListWidget>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    db(std::make_shared<Database>()),
    tm(std::make_unique<TaskManager>(db))
{
    ui->setupUi(this);
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Find section
    QVBoxLayout *findLayout = new QVBoxLayout;
    selectBoxLabel = new QLabel("Select Stored Box:", this);

    findComboBox = new QComboBox(this);
    findLayout->addWidget(new QLabel("Find:", this));
    findLayout->addWidget(selectBoxLabel);
    findLayout->addWidget(findComboBox);

    findButton = new QPushButton("Find", this);
    findButton->setEnabled(false); // Initially disable the button
    findLayout->addWidget(findButton);

    // Add section
    QVBoxLayout *addLayout = new QVBoxLayout;
    selectBoxLabel2 = new QLabel("Select Known Box:", this);

    boxComboBox = new QComboBox(this);
    selectTrayLabel = new QLabel("Select Tray:", this);
    trayComboBox = new QComboBox(this);

    addLayout->addWidget(new QLabel("Add:", this));
    addLayout->addWidget(selectBoxLabel2);
    addLayout->addWidget(boxComboBox);
    addLayout->addWidget(selectTrayLabel);
    addLayout->addWidget(trayComboBox);

    addButton = new QPushButton("Add", this);
    addButton->setEnabled(false); // Initially disable the button
    addLayout->addWidget(addButton);

    // Slider for tolerances
    toleranceSlider = new QSlider(Qt::Horizontal, this);
    toleranceSlider->setMinimum(0);
    toleranceSlider->setMaximum(100);
    toleranceSlider->setValue(50);
    toleranceSlider->setTickInterval(10);
    toleranceSlider->setTickPosition(QSlider::TicksBelow);

    // Layout for tolerances
    toleranceLayout = new QHBoxLayout;
    toleranceLayout->addWidget(new QLabel("Tolerances:", this));
    toleranceLayout->addWidget(toleranceSlider);

    QHBoxLayout *trayButtonLayout = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        trayButtons[i] = new QPushButton(QString("Tray %1").arg(i + 1), this);
        trayButtonLayout->addWidget(trayButtons[i]);
        connect(trayButtons[i], &QPushButton::clicked, this, [=]() { trayButtonClicked(i + 1); });
    }

    // Add layouts to mainLayout
    mainLayout->addLayout(findLayout);
    mainLayout->addLayout(addLayout);
    mainLayout->addLayout(toleranceLayout);
    mainLayout->addLayout(trayButtonLayout);

    // Connect findComboBox to enable/disable the findButton
    connect(findComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        findButton->setEnabled(!findComboBox->currentText().isEmpty() && findComboBox->currentText() != "");
    });

    // Connect box and tray selection to enable/disable the addButton
    connect(boxComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        addButton->setEnabled(!boxComboBox->currentText().isEmpty() && !trayComboBox->currentText().isEmpty() && boxComboBox->currentText() != "");
    });
    connect(trayComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        addButton->setEnabled(!boxComboBox->currentText().isEmpty() && !trayComboBox->currentText().isEmpty() && trayComboBox->currentText() != "");
    });


    // Connect addButton to addButtonClicked() only when both box and tray are selected
    connect(findButton, &QPushButton::clicked, this, &MainWindow::findButtonClicked);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addButtonClicked);
    trayTimer = new QTimer(this);
    trayTimer->setSingleShot(true);
    populateBoxLists();

    populateBoxLists();




}

MainWindow::~MainWindow()
{
}


// Function to populate a QComboBox with items from a QList<int>
void MainWindow::populateBoxLists() {

    // Clear existing items in QList
    addList.clear();
    findList.clear();
    trayList.clear();
    findComboBox->addItem(""); // Add an empty item to findComboBox
    boxComboBox->addItem("");  // Add an empty item to boxComboBox
    trayComboBox->addItem(""); // Add an empty item to trayComboBox

    notStored = db->getUnstoredBoxes();
    storedBoxes = db->getStoredBoxes();

     // Clear existing items in QList
    for (int boxId : notStored) {
        addList.append(boxId);
        std::cout << boxId << std::endl;
    }
    for (int boxId : storedBoxes) {
        findList.append(boxId);
        std::cout << boxId << std::endl;
    }
    for (int i = 0; i <= 5; ++i) {
        trayList.append(i);
    }
    for (int boxId : findList) {
        findComboBox->addItem(QString::number(boxId));
    }
    for (int boxId : addList) {
        boxComboBox->addItem(QString::number(boxId));
    }
    for (int trayId : trayList) {
        trayComboBox->addItem(QString::number(trayId));
    }
}



void MainWindow::findButtonClicked()
{
    QString id = findComboBox->currentText();
    // Perform action with the entered ID
    QMessageBox::information(this, "Find", QString("Find button clicked with ID: %1").arg(id));

    int tray = db->getTrayId(id.toInt());
    std::cout << tray << std::endl;

    db->addTask(id.toInt(),2,tray);
    if(tray == dockedTray && tray == 0)
     {
            Task task = Task(0, 1,id.toInt(),dockedTray);

           db->addTask(id.toInt(),1,dockedTray);}



}

void MainWindow::addButtonClicked()
{
    QString id = boxComboBox->currentText();
    QString tray = trayComboBox->currentText();

    QMessageBox::information(this, "Add", QString("Add box with id %1").arg(id));
    Database test = Database();
    std::cout << id.toInt() << std::endl;


     std::cout << "arrived" << std::endl;

     if(tray.toInt() == dockedTray && tray.toInt() == 0)
     {
        Task task = Task(0, 1,id.toInt(),dockedTray);
        db->addTask(id.toInt(),1,dockedTray);
     }
     else
     {

         db->addTask(id.toInt(),1,tray.toInt());
     }


}

void MainWindow::trayButtonClicked(int trayNumber){
    std::cout << "tray docked" << std::endl;
    dockedTray = trayNumber;
    QMessageBox::information(this, "Tray", QString("Tray %1 ").arg(trayNumber));

    trayTimer->start(15000); // 15 seconds
    tm->prepTasks(trayNumber);

}

