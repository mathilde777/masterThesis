#include "TaskManager.h"
#include <iostream>


TaskManager::TaskManager(std::shared_ptr<Database> db) : db(db), taskExecuting(false), donePreparing(false) {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
    knownBoxes = db->getKnownBoxes();
     updateTask = std::make_shared<UpdateTask>(db);
     connect(updateTask.get(), &UpdateTask::taskCompleted, this, &TaskManager::onTaskCompleted);
     connect(updateTask.get(), &UpdateTask::updateStatus, this, &TaskManager::updateUiStatus);
     addTask = std::make_shared<AddTask>(db);
     connect(addTask.get(), &AddTask::taskCompleted, this, &TaskManager::onTaskCompleted);

     findTask = std::make_shared<FindTask>(db,knownBoxes);
     connect(findTask.get(), &FindTask::taskCompleted, this, &TaskManager::onTaskCompleted);
     connect(findTask.get(), &FindTask::updateStatus, this, &TaskManager::updateUiStatus);

}

TaskManager::~TaskManager() {
    disconnect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
}

void TaskManager::updateKnownBoxes() {
    knownBoxes = db->getKnownBoxes();
}

void TaskManager::prepTasks(int id) {
    std::cout << "about to make the tasks" << std::endl;
    QThread* thread = new QThread;
    tray = id;
    trayBoxes = db->getAllBoxesInTray(id);
    TaskPreparer* preparer = new TaskPreparer(id, db);

    preparer->moveToThread(thread);

    connect(preparer, &TaskPreparer::taskPrepared, this, &TaskManager::onTaskPrepared);
    connect(thread, &QThread::started, preparer, &TaskPreparer::prepareTask);
    connect(preparer, &TaskPreparer::finished, this, &TaskManager::preparingDone);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

bool TaskManager::checkFlaggedBoxes(int productId) {
    return TaskFunctions::checkFlaggedBoxes(productId, knownBoxes);
}

void TaskManager::preparingDone() {
    std::cout << "DONE PREPARING" << std::endl;
    donePreparing = true;
}

void TaskManager::trayDocked() {
    std::cout << "tray docked executing tasks" << std::endl;
    refernce = db->getReferences(0); // this is still to be furthur developped so that depending on the tray position a different reference point is passed
    startExecutionLoop();
    emit trayDockedUpdate();
}

void TaskManager::onTaskPrepared(std::shared_ptr<Task> task) {
    executingQueue.push_back(task);
    std::cout << "task prepped and added to execute" << std::endl;
    emit taskPrepared();
}

void TaskManager::startExecutionLoop() {
    while (!executingQueue.empty() || !donePreparing) {
        if (!executingQueue.empty()) {
            executeTasks();
        } else if (!donePreparing) {
            waitForTasks();
        }
    }

    executeUpdateTask(tray);
    std::cout << "DONE EXECUTING TASKS" << std::endl;
}

void TaskManager::waitForTasks() {
    std::cout << "waiting for TASKS" << std::endl;
    QEventLoop loop;
    connect(this, &TaskManager::taskPrepared, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(this, &TaskManager::taskPrepared, &loop, &QEventLoop::quit);
}

void TaskManager::onTaskCompleted() {
    std::cout << "task done" << std::endl;
    emit refresh();
    taskExecuting = false;
}

void TaskManager::executeTasks() {
    if (taskExecuting || executingQueue.empty()) {
        return;
    }

    auto task = executingQueue.front();
    taskExecuting = true;

    switch (task->getType()) {
    case 1:
        executeAddTask(task);
        break;
    case 0:
        executeFindTask(task);
        break;
    default:
        std::cout << "UNKNOWN TASK " << std::endl;
        break;
    }
}

void TaskManager::executeAddTask(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("ADD: start to Add box with id %1").arg(task->getBoxId()));
    std::cout << "adding box" << std::endl;
    addTask->execute(task);
    //db->storeBox(task->getBoxId(), task->getTray());
    removeExecutedTask(task);
}

void TaskManager::executeFindTask(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("FIND: start to find box with id %1").arg(task->getBoxId()));
    std::cout << "finding box" << std::endl;
    findTask->execute(task,refernce);

    removeExecutedTask(task);
}

void TaskManager::executeUpdateTask(int trayid) {

    std::cout << "UPDATE" << std::endl;
    updateTask->execute(trayid,refernce);

}
void TaskManager::removeExecutedTask(const std::shared_ptr<Task>& task) {
    db->removeTaskFromQueue(task->getId());
    executingQueue.pop_front();
    emit taskCompleted();
}

void TaskManager::updateUiStatus(const QString& message)
{
     emit updateStatus(message);
}

//calibrate runs the calibrate function in Detectin3D and allows the system to consider that the tray can arrive at 1 of 4 position which affects its height to the camera.
void TaskManager::calibrateTray(int position, double height)
{
    auto refPoint = calibrate(height);
    db->addReference(position,refPoint.x(),refPoint.y(),refPoint.z());
    emit updateStatus(QString("CALIBRATING TRAY"));
}
