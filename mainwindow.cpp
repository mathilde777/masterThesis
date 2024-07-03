#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Detection2D.h"
#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QDateTime>
#include <QFormLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QSpacerItem>
#include <QFileDialog>


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

    // Docked Info Label
    dockedInfoLabel = new QLabel("No tray docked", this);
    mainLayout->addWidget(dockedInfoLabel);

    // Status Text
    statusText = new QTextEdit(this);
    statusText->setReadOnly(true);
    mainLayout->addWidget(statusText);

    // Find Section
    QGroupBox *findGroupBox = new QGroupBox("Find Stored Box");
    QVBoxLayout *findLayout = new QVBoxLayout(findGroupBox);
    selectBoxLabel = new QLabel("Select Stored Box:", this);
    findComboBox = new QComboBox(this);
    findLayout->addWidget(selectBoxLabel);
    findLayout->addWidget(findComboBox);
    findButton = new QPushButton("Find", this);
    findButton->setEnabled(false);
    findLayout->addWidget(findButton);
    mainLayout->addWidget(findGroupBox);

    // Add Section
    QGroupBox *addGroupBox = new QGroupBox("Add Known Box to Tray");
    QVBoxLayout *addLayout = new QVBoxLayout(addGroupBox);
    selectBoxLabel2 = new QLabel("Select Known Box:", this);
    boxComboBox = new QComboBox(this);
    selectTrayLabel = new QLabel("Select Tray:", this);
    trayComboBox = new QComboBox(this);
    addLayout->addWidget(selectBoxLabel2);
    addLayout->addWidget(boxComboBox);
    addLayout->addWidget(selectTrayLabel);
    addLayout->addWidget(trayComboBox);
    addButton = new QPushButton("Add", this);
    addButton->setEnabled(false);
    addLayout->addWidget(addButton);
    mainLayout->addWidget(addGroupBox);

    // Run Tasks Section
    QGroupBox *runTasksGroupBox = new QGroupBox("Run Tray Tasks");
    QVBoxLayout *runTasksLayout = new QVBoxLayout(runTasksGroupBox);
    runTrayTasksComboBox = new QComboBox(this);
    for (int i = 0; i < 15; ++i) {
        runTrayTasksComboBox->addItem(QString("Tray %1").arg(i + 1));
    }
    runTasksLayout->addWidget(runTrayTasksComboBox);
    runTasksButton = new QPushButton("Run Tasks", this);
    runTasksLayout->addWidget(runTasksButton);
    mainLayout->addWidget(runTasksGroupBox);

    // Update Section
    QGroupBox *updateGroupBox = new QGroupBox("Update Docked Tray");
    QVBoxLayout *updateLayout = new QVBoxLayout(updateGroupBox);
    updateButton = new QPushButton("Update docked tray", this);
    updateLayout->addWidget(updateButton);
    mainLayout->addWidget(updateGroupBox);

    // Calibration Section
    QGroupBox *calibrateGroupBox = new QGroupBox("Calibrate Tray");
    QVBoxLayout *calibrateLayout = new QVBoxLayout(calibrateGroupBox);
    calibration = new QPushButton("Calibrate tray", this);
    calibrateLayout->addWidget(calibration);
    mainLayout->addWidget(calibrateGroupBox);

    // Add New Box Section
    QGroupBox *addNewBoxGroupBox = new QGroupBox("Add New Known Box");
    QVBoxLayout *addNewBoxLayout = new QVBoxLayout(addNewBoxGroupBox);
    addNewBox = new QPushButton("Add new Known Box", this);
    addNewBoxLayout->addWidget(addNewBox);
    mainLayout->addWidget(addNewBoxGroupBox);

    // Add Training Images Section
    QGroupBox *addTrainingImagesGroupBox = new QGroupBox("Add Images for 2D Training");
    QVBoxLayout *addTrainingImagesLayout = new QVBoxLayout(addTrainingImagesGroupBox);
    addTrainingImages = new QPushButton("Add Images for 2D training", this);
    addTrainingImagesLayout->addWidget(addTrainingImages);
    mainLayout->addWidget(addTrainingImagesGroupBox);

    // Adding spacing between sections
    mainLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Connect signals and slots
    connect(findComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        findButton->setEnabled(!findComboBox->currentText().isEmpty());
    });
    connect(boxComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        addButton->setEnabled(!boxComboBox->currentText().isEmpty() && !trayComboBox->currentText().isEmpty());
    });
    connect(trayComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        addButton->setEnabled(!boxComboBox->currentText().isEmpty() && !trayComboBox->currentText().isEmpty());
    });

    connect(findButton, &QPushButton::clicked, this, &MainWindow::findButtonClicked);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addButtonClicked);
    connect(runTasksButton, &QPushButton::clicked, this, &MainWindow::trayButtonClicked);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateButtonClicked);
    connect(calibration, &QPushButton::clicked, this, &MainWindow::calibrate);
    connect(addNewBox, &QPushButton::clicked, this, &MainWindow::newBoxClicked);
    connect(addTrainingImages, &QPushButton::clicked, this, &MainWindow::addImages);

    trayTimer = new QTimer(this);
    trayTimer->setSingleShot(true);
    connect(trayTimer, &QTimer::timeout, tm.get(), &TaskManager::trayDocked);
    connect(tm.get(), &TaskManager::refresh, this, &MainWindow::populateBoxLists);
    connect(tm.get(), &TaskManager::trayDockedUpdate, this, &MainWindow::updateDockedInfo);
    connect(tm.get(), &TaskManager::updateStatus, this, &MainWindow::updateStatusText);

    populateBoxLists();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::populateBoxLists() {
    findComboBox->clear();
    boxComboBox->clear();
    trayComboBox->clear();

    findComboBox->addItem(""); // Add an empty item to findComboBox
    boxComboBox->addItem("");  // Add an empty item to boxComboBox
    trayComboBox->addItem(""); // Add an empty item to trayComboBox

    storedBoxes.clear();
    notStored = db->getKnownBoxes();
    storedBoxes = db->getStoredBoxes();

    for (const auto& box : storedBoxes) {
        int boxId = box.first;
        std::string boxName = box.second;
        findComboBox->addItem(QString("%1 - %2").arg(boxId).arg(QString::fromStdString(boxName)));
    }
    for (const auto& box : notStored) {
        if (!(box->trained)) {
            boxComboBox->addItem(QString("%1 - %2 - not trained yet").arg(box->getProductId()).arg(QString::fromStdString(box->getProductName())));
        } else {
            boxComboBox->addItem(QString("%1 - %2").arg(box->getProductId()).arg(QString::fromStdString(box->getProductName())));
        }
    }
    for (int trayId = 0; trayId <= 15; ++trayId) {
        trayComboBox->addItem(QString::number(trayId));
    }
}

void MainWindow::findButtonClicked() {
    QString selectedText = findComboBox->currentText();
    QStringList parts = selectedText.split(" - ");
    if (parts.size() == 2) {
        int id = parts[0].toInt();
        int tray = db->getTrayId(id);
         QMessageBox::information(this, "Find", QString("Find button clicked with ID: %1").arg(id));
        db->addTask(id, 0, tray);
    } else {
        QMessageBox::warning(this, "Error", "Invalid selection");
    }
}

void MainWindow::addButtonClicked() {
    QString idText = boxComboBox->currentText();
    QStringList parts = idText.split(" - ");
    if (parts.size() == 2) {
        int id = parts[0].toInt();
        int tray = trayComboBox->currentText().toInt();
        QMessageBox::information(this, "Add", QString("Add box with product id %1").arg(id));

        if (tray == dockedTray && tray == 0) {
            db->addTask(id, 1, dockedTray);
        } else {
            db->addTask(id, 1, tray);
        }
    } else {
        QMessageBox::warning(this, "Error", "Invalid selection");
    }
}

void MainWindow::trayButtonClicked() {
    int trayNumber = runTrayTasksComboBox->currentIndex()+1;
    dockedTray = trayNumber;
    QMessageBox::information(this, "Tray", QString("Tray %1 ").arg(trayNumber));
    dockedInfoLabel->setText(QString("Tray %1 docking").arg(trayNumber));
    trayTimer->start(100);
    tm->prepTasks(trayNumber);
}

void MainWindow::updateDockedInfo() {
    dockedInfoLabel->setText(QString("Tray %1 docked").arg(dockedTray));
}

void MainWindow::updateStatusText(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ");
    statusText->append(timestamp + message);
}

void MainWindow::updateButtonClicked() {
    if (dockedTray != 0) {
        tm->executeUpdateTask(dockedTray);
    } else {
        QMessageBox::warning(this, "Error", "No tray docked");
    }
}

void MainWindow::newBoxClicked() {
    createInputDialog();
}

void MainWindow::createInputDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Input Required");

    QFormLayout formLayout(&dialog);
    QLineEdit lineEditWidth, lineEditHeight, lineEditLength, lineEditName;
    formLayout.addRow("Width:", &lineEditWidth);
    formLayout.addRow("Height:", &lineEditHeight);
    formLayout.addRow("Length:", &lineEditLength);
    formLayout.addRow("Name:", &lineEditName);

    QPushButton okButton("OK"), cancelButton("Cancel");
    formLayout.addRow(&okButton, &cancelButton);

    connect(&okButton, &QPushButton::clicked, [&]() {
        QString width = lineEditWidth.text();
        QString height = lineEditHeight.text();
        QString length = lineEditLength.text();
        QString name = lineEditName.text();
        dialog.accept();
        addNewKnownBox(width, height, length, name);
    });

    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    dialog.exec();
}

void MainWindow::addNewKnownBox(const QString &width, const QString &height, const QString &length, const QString &name) {
    if (width.isEmpty() || height.isEmpty() || length.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields should have a value.");
    } else {
        db->newKnownBox(name.toStdString(), width.toDouble(), height.toDouble(), length.toDouble());
        populateBoxLists();
        tm->updateKnownBoxes();
    }
}

void MainWindow::calibrate() {
    std::array<double, 4> positions = {690, 500, 400, 700};
    int positionIndex = 1;


    tm->calibrateTray(positionIndex - 1,positions[positionIndex-1]);


}

void MainWindow::addImages()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add Training Images");

    QFormLayout formLayout(&dialog);

    QLabel label("Select Untrained Known Box:");
    QComboBox comboBox;
    QPushButton fileButton("Choose Image File");
    QLabel fileNameLabel("No file chosen");

    // Populate comboBox with untrained known boxes
    for (const auto& box : notStored) {
        if (!box->trained) {
            comboBox.addItem(QString("%1 - %2").arg(box->getProductId()).arg(QString::fromStdString(box->getProductName())));
        }
    }

    formLayout.addRow(&label, &comboBox);
    formLayout.addRow(&fileButton, &fileNameLabel);

    QPushButton okButton("OK");
    QPushButton cancelButton("Cancel");

    formLayout.addRow(&okButton, &cancelButton);

    // File dialog connection
    connect(&fileButton, &QPushButton::clicked, [&]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Open Image File", "", "Images (*.png *.xpm *.jpg)");
        if (!fileName.isEmpty()) {
            fileNameLabel.setText(fileName);
        }
    });

    connect(&okButton, &QPushButton::clicked, [&]() {
        QString selectedBox = comboBox.currentText();
        QString fileName = fileNameLabel.text();

        if (selectedBox.isEmpty() || fileName == "No file chosen") {
            QMessageBox::warning(this, "Input Error", "Please select a known box and an image file.");
        } else {
            dialog.accept();
            processTrainingImage(selectedBox, fileName);
        }
    });

    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MainWindow::processTrainingImage(const QString& selectedBox, const QString& fileName)
{
    QStringList parts = selectedBox.split(" - ");
    int id = parts[0].toInt();
    std::string fileNameStd = fileName.toStdString();
    db->addTrainingImage(id, fileNameStd);
    QMessageBox::information(this, "Success", QString("Added training image for box ID %1: %2").arg(id).arg(fileName));
}


