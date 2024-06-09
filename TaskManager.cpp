#include "TaskManager.h"
#include <iostream>
#include "FindTask.h"
#include "UpdateTask.h"

TaskManager::TaskManager(std::shared_ptr<Database> db) : db(db), taskExecuting(false), donePreparing(false), noResults(false) {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);
    knownBoxes = db->getKnownBoxes();
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
    UpdateTask updateTask(tray);
    updateTask.execute(db);
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
    noResults = false;
    if (taskExecuting || executingQueue.empty()) {
        return;
    }

    auto task = executingQueue.front();
    taskExecuting = true;

    switch (task->getType()) {
    case 1:
        executeAddBoxTask(task);
        break;
    case 0:
        executeFindBoxTask(task);
        break;
    default:
        std::cout << "UNKNOWN TASK " << std::endl;
        break;
    }
}

void TaskManager::executeAddBoxTask(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("ADD: start to Add box with id %1").arg(task->getBoxId()));
    std::cout << "adding box" << std::endl;
    db->storeBox(task->getBoxId(), task->getTray());
    removeExecutedTask(task);
}

void TaskManager::executeFindBoxTask(const std::shared_ptr<Task>& task) {
    emit updateStatus(QString("FIND: start to find box with id %1").arg(task->getBoxId()));
    std::cout << "finding box" << std::endl;
    FindTask findTask(task);
    findTask.execute(db);
    removeExecutedTask(task);
}

void TaskManager::removeExecutedTask(const std::shared_ptr<Task>& task) {
    db->removeTaskFromQueue(task->getId());
    executingQueue.pop_front();
    emit taskCompleted();
}
