#include "database.h"
#include "task.h"
#include <cppconn/prepared_statement.h>
#include <iostream>

#include <vector>

#define EXAMPLE_HOST "tcp://127.0.0.1:3306"
#define EXAMPLE_USER "root"
#define EXAMPLE_PASS "linux123"
#define EXAMPLE_DB "thesis"

//CONNECTION TO DATABASE


Database::Database() {
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        if (!driver) {
            std::cerr << "Failed to get MySQL driver instance" << std::endl;
            return;
        }
        con = driver->connect(EXAMPLE_HOST, EXAMPLE_USER, EXAMPLE_PASS);
        if (!con) {
            std::cerr << "Failed to connect to MySQL database" << std::endl;
            return;
        }
        con->setSchema("thesis");
        std::cout << "Connected to database 'thesis'" << std::endl;

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}

Database::~Database() {
    delete con;
}

void Database::addTask(int box_id, int task_type, int tray_id ) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL add_to_queue(?, ?, ?)"));
        pstmt->setInt(1, box_id);
        pstmt->setInt(2, task_type);
        pstmt->setInt(3, tray_id);
        pstmt->execute();
        std::cout << "Task added to the queue with box id " << box_id << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}
void Database::newKnownBox(std::string name, double width, double height,double  lenght)
{
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL addNewKnownBox(?, ?, ?,?)"));
        pstmt->setString(1, name);
        pstmt->setDouble(2, width);
        pstmt->setDouble(3, height);
        pstmt->setDouble(4, height);
        pstmt->execute();
        std::cout << "new product added to the list: " << name << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    }
//std::vector<std::tuple<int, std::string, int, int>>
std::vector<std::shared_ptr<Task>> Database::getTasks(int tray_id) {
    std::vector<std::shared_ptr<Task>> tasks;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetTasksInQueueByTray(?)"));
        pstmt->setInt(1, tray_id);
        pstmt->execute();

        do {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            while (resultSet && resultSet->next()) {
                int taskId = resultSet->getInt("id");
                int taskType = resultSet->getInt("task");
                int boxId = resultSet->getInt("boxId");
                int taskTrayId = resultSet->getInt("trayId");

                // Store task information in a temporary object
                tasks.emplace_back(std::make_shared<Task>(taskId, taskType, boxId, tray_id));
            }
        } while (pstmt->getMoreResults()); // Consume additional result sets
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    return tasks;
}


void Database::updateBox(int id,double last_x, double last_y,double last_z,double cx, double cy,double cz) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL updateBox(?,?, ?, ?, ?,?,?)"));
        pstmt->setInt(1, id);
        pstmt->setDouble(2, last_x);
        pstmt->setDouble(3, last_y);
        pstmt->setDouble(4, last_z);
        pstmt->setDouble(5, cx);
        pstmt->setDouble(6, cy);
        pstmt->setDouble(7, cz);
        pstmt->execute();
        std::cout << "UPdating Box with box id " << id << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}


void Database::storeBox(int id, int tray) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL store_box(?, ?)"));
        pstmt->setInt(1, id);
        pstmt->setInt(2, tray);
        pstmt->execute();
        std::cout << "Box with ID " << id << " stored in tray " << tray << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    //emit taskCompleted();
}



bool Database::checkStoredBoxes( int boxId) {
    bool foundBox = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL check_stored_box(?, @found_box)"));
        pstmt->setInt(1, boxId);
        pstmt->execute();

        // Retrieve the output parameter value
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        sql::ResultSet* rs = stmt->executeQuery("SELECT @found_box");
        if (rs->next()) {
            foundBox = rs->getBoolean(1);
        }
        delete rs;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    if(!foundBox)
    {
        std::cout << "Box not stored in a tray" << std::endl;

    }
    return foundBox;
}

bool Database::checkKnownBoxes(int boxId) {
    bool foundBox = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL check_known_box(?, @found_box)"));
        pstmt->setInt(1, boxId);
        pstmt->execute();

        // Consume any potential result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());
            // Not expecting data here, just consume any possible result set
        }

        // Now, retrieve the output parameter value
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT @found_box AS found_box"));

        // Check the value of @found_box
        if (rs && rs->next()) {
            foundBox = rs->getBoolean("found_box"); // Use column label for clarity
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    if (!foundBox) {
        std::cout << "Box not known" << std::endl;
    }
    return foundBox;
}





void Database::removeStoredBox(int boxId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL RemoveStoredBox(?)"));
        pstmt->setInt(1, boxId);
        pstmt->execute();

        // Consume any potential result sets to avoid "Commands out of sync" error, even if not expected
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());
            // No need to process resultSet here, just ensuring it's consumed if present
        }

        std::cout << "Stored box with ID " << boxId << " removed successfully." << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}


// Execute MySQL stored procedure to get information about a box
std::tuple<int, double, double, double, double, double, double,Eigen::Vector3f> Database::getBoxInfo(int Id) {
    std::tuple<int, double, double, double, double, double, double,Eigen::Vector3f> boxInfo;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetBoxInfo(?)"));
        pstmt->setInt(1, Id);
        bool hasResults = pstmt->execute();

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            if (resultSet && resultSet->next()) {
                int boxId = resultSet->getInt(1);
                double width = resultSet->getDouble(2);
                double height = resultSet->getDouble(3);
                double length = resultSet->getDouble(4);
                double last_x = resultSet->getDouble(5);
                double last_y = resultSet->getDouble(6);
                double last_z = resultSet->getDouble(7);
                double clusterX = resultSet->getDouble(8);
                double clusterY = resultSet->getDouble(9);
                double clusterZ = resultSet->getDouble(10);
                boxInfo = std::make_tuple(boxId, width, height, length, last_x, last_y, last_z,Eigen::Vector3f(clusterX,clusterY,clusterZ));

            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return boxInfo;
}

// Execute MySQL stored procedure to check if a box with the same dimensions exists
bool Database::checkExistingBoxes(int tray_id, int box_id) {
    bool exists = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL CheckForSameDimensions(?, ?)"));
        pstmt->setInt(1, tray_id);
        pstmt->setInt(2, box_id);

        bool isResult = pstmt->execute(); // Execute the query

        if (isResult) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());
            exists = resultSet && resultSet->next();
        }

        // Consume any additional unexpected result sets
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    return exists;
}



void Database::removeTaskFromQueue(int taskId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL RemoveTaskFromQueue(?)"));
        pstmt->setInt(1, taskId);
        pstmt->execute();

        // Consume any potential result sets
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());
        }

        std::cout << "Task with ID " << taskId << " removed successfully." << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}

int Database::getTrayId(int box_id) {
    int id = 0;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetTrayIdByBoxId(?)"));
        pstmt->setInt(1, box_id);
        bool hasResults = pstmt->execute(); // Execute the stored procedure

        do {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            // Check if the result set is valid and process it
            if (resultSet && resultSet->next()) {
                id = resultSet->getInt("trayId");
            }
        } while (pstmt->getMoreResults()); // Consume any additional result sets

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return id;
}


// Add a new method to your Database class to call the stored procedure and retrieve the list of unstored box IDs
std::vector<int> Database::getUnstoredBoxes() {
    std::vector<int> unstoredBoxes;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetUnstoredBoxes()"));
        bool isResult = pstmt->execute(); // Execute the stored procedure

        if (isResult) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            while (resultSet && resultSet->next()) {
                int boxId = resultSet->getInt("id");
                unstoredBoxes.push_back(boxId);
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return unstoredBoxes;
}

// Add a new method to your Database class to call the stored procedure and retrieve the list of unstored box IDs
std::vector<std::shared_ptr<KnownBox>> Database::getKnownBoxes() {
  std::vector<std::shared_ptr<KnownBox>> knownBoxes = std::vector<std::shared_ptr<KnownBox>>();
    //std::vector<std::shared_ptr<KnownBox>> knownBoxes;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL allKnownBoxes()"));
        bool isResult = pstmt->execute(); // Execute the stored procedure

        if (isResult) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            while (resultSet && resultSet->next()) {
                int boxId = resultSet->getInt("id");
                std::string boxName = resultSet->getString("productName");
                int new_box = resultSet->getInt("newBox");
                int trained = resultSet->getInt("trained");

                  std::shared_ptr<KnownBox> box = std::make_shared<KnownBox>(boxId, boxName, new_box, trained);

                knownBoxes.push_back(box);
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return knownBoxes;
}


std::vector<std::pair<int, std::string>> Database::getStoredBoxes() {
    std::vector<std::pair<int, std::string>> storedBoxes;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL allStoredBoxes()"));
        bool isResult = pstmt->execute(); // Execute the stored procedure

        if (isResult) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            while (resultSet && resultSet->next()) {
                int id = resultSet->getInt("id");
                int boxId = resultSet->getInt("boxId");
                std::string boxName = resultSet->getString("productName");
                storedBoxes.push_back(std::make_pair(id, boxName));
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return storedBoxes;
}

std::tuple<double, double, double> Database::getBoxDimensions(int Id) {
    std::tuple<double, double, double> dimensions;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetBoxDimensions(?)"));
        pstmt->setInt(1, Id);
        bool hasResults = pstmt->execute(); // Execute the stored procedure

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            if (resultSet && resultSet->next()) {
                double width = resultSet->getDouble("width");
                double height = resultSet->getDouble("height");
                double length = resultSet->getDouble("length");
                dimensions = std::make_tuple(width, height, length);
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return dimensions;
}


std::shared_ptr<Box> Database::getBox(int tray, int Id) {
    std::shared_ptr<Box> box;

    try {
        auto boxInfo = getBoxInfo(Id);
        int boxId = std::get<0>(boxInfo);
        double width = std::get<1>(boxInfo);
        double height = std::get<2>(boxInfo);
        double length = std::get<3>(boxInfo);
        double last_x = std::get<4>(boxInfo);
        double last_y = std::get<5>(boxInfo);
        double last_z = std::get<6>(boxInfo);
        Eigen::Vector3f clusterD = std::get<7>(boxInfo);

        box = std::make_shared<Box>(Id, boxId, tray, last_x, last_y, last_z, width, height, length);
        box->setCluster(clusterD);
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return box;
}

std::vector<std::shared_ptr<Box>> Database::getAllBoxesInTray(int trayId) {
    std::vector<std::shared_ptr<Box>> boxes;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetAllBoxesInTray(?)"));
        pstmt->setInt(1, trayId);
        bool hasResults = pstmt->execute(); // Execute the stored procedure

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            while (resultSet && resultSet->next()) {
                int id = resultSet->getInt("id");
                int boxId = resultSet->getInt("boxId");
                int trayId = resultSet->getInt("trayId");
                double lastX = resultSet->getDouble("lastX");
                double lastY = resultSet->getDouble("lastY");
                double lastZ = resultSet->getDouble("lastZ");
                double width = resultSet->getDouble("width");
                double height = resultSet->getDouble("height");
                double length = resultSet->getDouble("length");
                double clusterX = resultSet->getDouble("clusterX");
                double clusterY = resultSet->getDouble("clusterY");
                double clusterZ = resultSet->getDouble("clusterZ");


                // Create a new Box object with the retrieved information
                auto box = std::make_shared<Box>(id, boxId, trayId, lastX, lastY, lastZ, width, height, length);
                box->setCluster(Eigen::Vector3f(clusterX,clusterY,clusterZ));
                boxes.push_back(box);
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return boxes;
}

