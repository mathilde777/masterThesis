﻿#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "detection2D.h"
#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dockedTray(0),
    ui(new Ui::MainWindow),
    db(std::make_shared<Database>()),
    tm(std::make_unique<TaskManager>(db))
{
    ui->setupUi(this);
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    dockedInfoLabel = new QLabel("No tray docked", this); // Initialize the QLabel
    mainLayout->addWidget(dockedInfoLabel);

    statusText = new QTextEdit(this);
    statusText->setReadOnly(true); // Make it read-only
    mainLayout->addWidget(statusText);
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
    // Assuming tm is a std::unique_ptr<TaskManager>
    connect(trayTimer, &QTimer::timeout, tm.get(), &TaskManager::trayDocked);
    connect(tm.get(), &TaskManager::refresh, this, &MainWindow::populateBoxLists);
    connect(tm.get(), &TaskManager::trayDockedUpdate, this, &MainWindow::updateDockedInfo);
    //connect(tm.get(), &TaskManager::errorOccurredTask, this, &MainWindow::handleErrorTask);
    //connect(tm.get(), &TaskManager::errorOccurredUpdate, this, &MainWindow::handleErrorUpdate);
    connect(tm.get(), &TaskManager::updateStatus, this, &MainWindow::updateStatusText);

    updateButton = new QPushButton("Update", this);
/
    // Add the Update button to the layout
    mainLayout->addWidget(updateButton);

    // Connect the clicked signal of the Update button to a slot that contains the update function logic
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateButtonClicked);


    calibration = new QPushButton("Calibrate", this);

    // Add the Update button to the layout
    mainLayout->addWidget(calibration);

    // Connect the clicked signal of the Update button to a slot that contains the update function logic
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::calibrate);

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
    ;

    findComboBox->clear();
    boxComboBox->clear();
    trayComboBox->clear();

    findComboBox->addItem(""); // Add an empty item to findComboBox
    boxComboBox->addItem("");  // Add an empty item to boxComboBox
    trayComboBox->addItem(""); // Add an empty item to trayComboBox

    notStored.clear();
    storedBoxes.clear();
    notStored = db->getKnownBoxes();
    storedBoxes = db->getStoredBoxes();

    for (const auto& box :storedBoxes) {
        int boxId = box.first;
        std::string boxName = box.second;
        findComboBox->addItem(QString("%1 - %2").arg(boxId).arg(QString::fromStdString(boxName)));
    }
    for (const auto& box : notStored ) {
        int boxId = box.first;
        std::string boxName = box.second;
        boxComboBox->addItem(QString("%1 - %2").arg(boxId).arg(QString::fromStdString(boxName)));
    }
    for (int trayId = 0; trayId <= 5; ++trayId) {
        trayComboBox->addItem(QString::number(trayId));
    }
}



void MainWindow::findButtonClicked()
{
    QString selectedtext = findComboBox->currentText();
    QMessageBox::information(this, "Find", QString("Find button clicked with ID: %1").arg(selectedtext));
    QStringList parts = selectedtext.split(" - "); // Split the text using the delimiter "-"
    if (parts.size() == 2) {
        QString idStr = parts[0]; // Extract the first part which should be the ID
        int id = idStr.toInt(); // Convert the ID string to an integer
        QMessageBox::information(this, "Find", QString("Find button clicked with ID: %1").arg(id));
        std::cout << "find idsssssssss" << id <<std::endl;
        int tray = db->getTrayId(id);
        db->addTask(id, 0, tray); // Assuming 0 is the task type for finding
    } else {
        QMessageBox::warning(this, "Error", "Invalid selection");
    }
}

void MainWindow::addButtonClicked()
{
    QString idText = boxComboBox->currentText();
    QStringList parts = idText.split(" - "); // Split the text using the delimiter "-"
    if (parts.size() == 2) {
        QString idStr = parts[0]; // Extract the first part which should be the ID
        int id = idStr.toInt(); // Convert the ID string to an integer

        QString tray = trayComboBox->currentText();

        QMessageBox::information(this, "Add", QString("Add box with product id %1").arg(id));
        std::cout << id << std::endl;

        std::cout << "arrived" << std::endl;

        if (tray.toInt() == dockedTray && tray.toInt() == 0)
        {

            db->addTask(id, 1, dockedTray);
            std::cout << "Adding task " << id << " to tray " << dockedTray << std::endl;
        }
        else
        {
            db->addTask(id, 1, tray.toInt());
            std::cout << "Adding task " << id << " to tray " << tray.toInt() << std::endl;
        }
    } else {
        QMessageBox::warning(this, "Error", "Invalid selection");
    }

}

void MainWindow::trayButtonClicked(int trayNumber){
    std::cout << "tray docking" << std::endl;
    dockedTray = trayNumber;
    QMessageBox::information(this, "Tray", QString("Tray %1 ").arg(trayNumber));
    QString info = QString("Tray %1 docking").arg(trayNumber);
    dockedInfoLabel->setText(info);

    //trayTimer->start(15000); // 15 seconds
    trayTimer->start(100);
    tm->prepTasks(trayNumber);

}

void MainWindow::updateDockedInfo() {
    QString info = QString("Tray %1 docked").arg(dockedTray);
    dockedInfoLabel->setText(info);
}

void MainWindow::updateStatusText(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ");
    statusText->append(timestamp + message);
}

void MainWindow::updateButtonClicked() {

    if(dockedTray != 0)
    {
         tm->update(dockedTray);
    }
    else
    {
          QMessageBox::warning(this, "Error", "No tray docked");
    }
}


void MainWindow::addingNewKnownBox() {


}

void MainWindow::calibrate() {


}


/**
void MainWindow::handleErrorTask(QString errorMessage, int taskId) {
    // Display error message
    QMessageBox::critical(this, "Error", errorMessage);

    // Ask user to fix the error
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Fix the error?", "Do you want to fix the error and retry the task?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Retry the task associated with the error
        tm->retryTask(taskId);
    } else {
        // Skip the task and continue
        tm->skipTask(taskId);
    }
}

void MainWindow::handleErrorUpdate(QString errorMessage, int taskId) {
    // Display error message
    QMessageBox::critical(this, "Error", errorMessage);

    // Ask user to fix the error
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Fix the error?", "Do you want to fix the error and retry the task?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Retry the task associated with the error
        tm->retryTask(taskId);
    } else {
        // Skip the task and continue
        tm->skipTask(taskId);
    }
}**/
