
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
    trayBoxes = db->getAllBoxesInTray(id);
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
        findBoxesOfSameSize(*task->getBox());
        taskExecuting = true;
        if (task->getType() == 1) {
            std::cout << "adding box" << std::endl;
            db->storeBox(task->getBoxId(), task->getTray());

            //RUN UPDATE

        }
        else if(task->getType()==0)
        {
            if (!possibleSameSize.empty()) {
                //here is when there could be possible of 2 smae size...... not certain depending on the rorentaiton in the iamges
                std::cout << "RUN 3D imaging" << std::endl;

               std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection();
               std::shared_ptr<std::vector<std::pair<ClusterInfo, double>>> results = matchClusterWithBox(resultsCluster, task->getBox());
               if(results->size() > 1)
                {
                    //check with 2 D
                   std::unique_ptr<DetectionResult> result2D = run2D("file path");

                }

               else if (results->empty())
               {
                   //problem
                    // RUN 3D WITH A BIGGER IMAGE
               }

                else if(results->size() == 1)
               {
                    //no need for 2D
               }


            std::cout << "task completed time to remove stored box" << task->getBoxId() << std::endl;
            db->removeStoredBox(task->getBoxId());

            // Asynchronously execute run3DDetectionThread using a lambda function
            /**
            // Do other tasks while waiting for the result...
            std::future<int> futureResult = std::async(std::launch::async, [this]() {
                return this->run3DDetectionThread();
            });

            // Get the result when it's available
            int result3D = futureResult.get();
**/

        }
        // Execute the task...

        // After the task is completed, emit the taskExecutionCompleted signal
        db->removeTaskFromQueue(executingQueue.front()->getId());
        executingQueue.erase(executingQueue.begin());
        emit taskCompleted();
    }

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

void TaskManager::update(int id)
{
    //list 3d
    //list 2d
    // list of boxes

    //comapres of the legnths of lists
    //if lagrger probelm
    //if smaller -> should be ok


    //get lists
    std::shared_ptr<std::vector<ClusterInfo>> resultsCluster = run3DDetection();


    //here run 2D.


}

void TaskManager::findBoxesOfSameSize(const Box& box1)

{
    std::vector<std::tuple<double, double, double>> dimensionPairs1 = {
        {box1.width, box1.height, box1.length},
        {box1.width, box1.length, box1.height},
        {box1.height, box1.width, box1.length},
        {box1.height, box1.length, box1.width},
        {box1.length, box1.width, box1.height},
        {box1.length, box1.height, box1.width}
    };
    for (const auto& box : trayBoxes) {
            std::vector<std::tuple<double, double, double>> dimensionPairs2 = {
                {box->width, box->height, box->length},
                {box->width, box->length, box->height},
                {box->height, box->width, box->length},
                {box->height, box->length, box->width},
                {box->length, box->width, box->height},
                {box->length, box->height, box->width}
            };



            bool foundMatch = false;
            if(box->boxId == box1.getBoxId())
            {
                foundMatch = true;
            }
            else{
            for (const auto& dim1 : dimensionPairs1) {
                for (const auto& dim2 : dimensionPairs2) {
                    if (dim1 == dim2 ) {
                        foundMatch = true;
                        break;
                    }
                }
                if (foundMatch) break;
            }

            }
            if (foundMatch) {
                possibleSameSize.push_back(box);
            }
        }


}
