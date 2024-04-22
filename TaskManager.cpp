
#include "TaskManager.h"
#include "database.h"
#include "detection2D.h"
#include "qeventloop.h"
#include "qtmetamacros.h"
#include "taskPreparer.h"
#include <QTimer>
#include <memory>
#include "3D_detection.h"
#include "result.h"
#include <future>


TaskManager::TaskManager(std::shared_ptr<Database> db) : db(db) {
    connect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);


    executionTimer = new QTimer(this);
  //  connect(executionTimer, &QTimer::timeout, this, &TaskManager::checkTasks);
    executionTimer->start(1000); // Adjust timeout interval as needed
    taskExecuting = false;
    donePreparing = false;
}



TaskManager::~TaskManager() {
    disconnect(this, &TaskManager::taskCompleted, this, &TaskManager::onTaskCompleted);

}


void TaskManager::prepTasks(int id)
{
    std::cout << "about to make the tasks"<< std::endl;
    QThread* thread = new QThread;

    TaskPreparer* preparer = new TaskPreparer(id,db);

    preparer->moveToThread(thread);

    connect(preparer, &TaskPreparer::taskPrepared, this, &TaskManager::onTaskPrepared);
    connect(thread, &QThread::started, preparer, &::TaskPreparer::prepareTask);

    connect(preparer, &TaskPreparer::finished, this, &TaskManager::preparingDone); // Corrected connection

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();


}


void TaskManager::preparingDone() {
    std::cout << "DONE PREPARING"<< std::endl;
    donePreparing = true; // Set the flag to true when the thread is finished preparing tasks
}

void TaskManager::trayDocked() {
     std::cout << "tray docked executing tasks"<< std::endl;
//executeTasks();
     startExecutionLoop();
     emit trayDockedUpdate();
}

void TaskManager::executeTasks() {
    if ( !taskExecuting) {
        auto task = executingQueue.front();
        // Set the taskExecuting flag to true to indicate that a task is being executed
        taskExecuting = true;
        if (task->getType() == 1) {
            std::cout << "adding box" << std::endl;
            db->storeBox(task->getBoxId(), task->getTray());

        }
        else if(task->getType()==0)
        {
            if (db->checkExistingBoxes(task->getTray(), task->getBoxId())) {
                std::cout << "RUN 3D imaging" << std::endl;
            } else {
                std::cout << "RUN 3D imaging" << std::endl;
            }

            std::cout << "task completed time to remove stored box" << task->getBoxId() << std::endl;
            db->removeStoredBox(task->getBoxId());

            // Asynchronously execute run3DDetectionThread using a lambda function
            std::future<int> futureResult = std::async(std::launch::async, [this]() {
                return this->run3DDetectionThread();
            });

            // Do other tasks while waiting for the result...

            // Get the result when it's available
            int result3D = futureResult.get();

        }
        // Execute the task...

        // After the task is completed, emit the taskExecutionCompleted signal
        db->removeTaskFromQueue(executingQueue.front()->getId());
        executingQueue.erase(executingQueue.begin());
        emit taskCompleted();
    }

}
int TaskManager::run3DDetectionThread() {

    auto result = run3DDetection(); // Assuming run3DDetection returns a Result

}

void TaskManager::onTaskPrepared(std::shared_ptr<Task> task) {
    executingQueue.push_back(task);
     std::cout << "task preped and added to execute" << std::endl;
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
     std::cout << "DONE EXECUTING TASKS" << std::endl;
}

void TaskManager::waitForTasks() {
     std::cout << "waiting for TASKS" << std::endl;
    QEventLoop loop;
    connect(this, &::TaskManager::taskPrepared, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(this, &::TaskManager::taskPrepared, &loop, &QEventLoop::quit);
}

void TaskManager::onTaskCompleted() {
     std::cout << "task done" << std::endl;
    emit refresh();
    taskExecuting = false;
}

void TaskManager::updateTray(int id)
{

}
