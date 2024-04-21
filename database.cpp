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


                auto box = getBox(tray_id,boxId);
                // Create Task object and associate Box
                auto newTask = std::make_shared<Task>(taskId, taskType, boxId, tray_id, box);
                tasks.push_back(newTask);
            }
        } while (pstmt->getMoreResults());
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    return tasks;
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
void Database::getBoxInfo(int boxId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetBoxInfo(?)"));
        pstmt->setInt(1, boxId);
        bool hasResults = pstmt->execute();

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());
            while (resultSet && resultSet->next()) {
                std::cout << "Box ID: " << resultSet->getInt("id") << std::endl;
                // Additional fields printed as before
            }
        }

        // Consume any additional unexpected result sets
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
            // No processing needed, just consuming to comply with protocol
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
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
                id = resultSet->getInt("trayid");
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
std::vector<std::pair<int, std::string>> Database::getKnownBoxes() {
    std::vector<std::pair<int, std::string>> knownBoxes;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL allKnownBoxes()"));
        bool isResult = pstmt->execute(); // Execute the stored procedure

        if (isResult) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            while (resultSet && resultSet->next()) {
                int boxId = resultSet->getInt("id");
                std::string boxName = resultSet->getString("productName");
                knownBoxes.push_back(std::make_pair(boxId, boxName));
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
                int boxId = resultSet->getInt("boxid");
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

std::shared_ptr<Box> Database::getBox(int tray,int Id) {
    std::shared_ptr<Box> box;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetLastKnownLocation(?)"));
        pstmt->setInt(1, Id);
        bool hasResults = pstmt->execute(); // Execute the stored procedure

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            if (resultSet && resultSet->next()) {
                int boxid = resultSet->getInt("boxid");
                double last_x = resultSet->getDouble("lastX");
                double last_y = resultSet->getDouble("lastY");
                double last_z = resultSet->getDouble("lastZ");

                // Retrieve box dimensions and last known location
                // Assuming you have functions to retrieve these values from the database
                double width, height, length;
                std::tie(width, height, length) = getBoxDimensions(Id);
                box = std::make_shared<Box>(Id, boxid, tray, last_x, last_y, last_z, width, height, length);
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
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
                int boxId = resultSet->getInt("boxid");
                int trayId = resultSet->getInt("trayid");
                double lastX = resultSet->getDouble("lastX");
                double lastY = resultSet->getDouble("lastY");
                double lastZ = resultSet->getDouble("lastZ");
                double width = resultSet->getDouble("width");
                double height = resultSet->getDouble("height");
                double length = resultSet->getDouble("length");

                // Create a new Box object with the retrieved information
                auto box = std::make_shared<Box>(id, boxId, trayId, lastX, lastY, lastZ, width, height, length);
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
