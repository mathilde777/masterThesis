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
        std::cout << "Task added to the queue" << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}
//std::vector<std::tuple<int, std::string, int, int>>
std::vector<Task> Database::getTasks(int tray_id) {
    std::vector<Task> tasks;
    try {
        // Prepare the statement to call the stored procedure
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetTasksInQueueByTray(?)"));
        pstmt->setInt(1, tray_id);

        // Execute the query
        sql::ResultSet* resultSet = pstmt->executeQuery();

        // Iterate over the result set and populate the vector of tasks
        while (resultSet->next()) {
            // Assuming Task constructor parameters are (int id, int type, int boxid, int tray)
            Task newTask(
                resultSet->getInt("id"),
                resultSet->getInt("task"),
                resultSet->getInt("boxId"),
                resultSet->getInt("trayId") // Use the correct column name here
                );
            tasks.push_back(newTask);
        }

        // Clean up
        delete resultSet;
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
        std::cout << "Value stored successfully" << std::endl;
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
        pstmt->clearParameters();
        stmt->close();
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    if(!foundBox)
    {
        std::cout << "Box not stored in a tray" << std::endl;

    }
    return foundBox;
}

bool Database::checkKnownBoxes( int boxId) {
    bool foundBox = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL check_known_box(?, @found_box)"));
        pstmt->setInt(1, boxId);
        pstmt->execute();

        // Retrieve the output parameter value
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        sql::ResultSet* rs = stmt->executeQuery("SELECT @found_box");
        if (rs->next()) {
            foundBox = rs->getBoolean(1);
        }
        delete rs;
        pstmt->clearParameters();
        stmt->close();
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    if(!foundBox)
    {
        std::cout << "Box not in database" << std::endl;

    }
    return foundBox;
}




// Execute MySQL stored procedure to get information about a box
void Database::getBoxInfo(int boxId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetBoxInfo(?)"));
        pstmt->setInt(1, boxId);
        sql::ResultSet* resultSet = pstmt->executeQuery();
        while (resultSet->next()) {
            std::cout << "Box ID: " << resultSet->getInt("id") << std::endl;
            std::cout << "Width: " << resultSet->getDouble("width") << std::endl;
            std::cout << "Height: " << resultSet->getDouble("height") << std::endl;
            std::cout << "Length: " << resultSet->getDouble("length") << std::endl;
            std::cout << "Last Position (X, Y, Z): " << resultSet->getInt("lastX") << ", " << resultSet->getInt("lastY") << ", " << resultSet->getInt("lastZ") << std::endl;
            std::cout << "Tray: " << resultSet->getInt("tray") << std::endl;
        }
        delete resultSet;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}
bool Database::checkExistingBoxes(int tray_id, int box_id) {
    bool exists = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL CheckForSameDimensions(?, ?)"));
        pstmt->setInt(1, tray_id);
        pstmt->setInt(2, box_id);

        // Execute the stored procedure
        sql::ResultSet* resultSet = pstmt->executeQuery();
        if (resultSet->next()) {
            exists = resultSet->getBoolean("exists_same_dimensions");
        }
        delete resultSet;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    return exists;
}

void Database::removeStoredBox(int boxId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL RemoveStoredBox(?)"));
        pstmt->setInt(1, boxId);
        pstmt->execute();
        std::cout << "Stored box with ID " << boxId << " removed successfully." << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}

void Database::removeTaskFromQueue(int taskId) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL RemoveTaskFromQueue(?)"));
        pstmt->setInt(1, taskId);
        pstmt->execute();

        std::cout << "Task with ID " << taskId << " removed successfully." << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}

int Database::getTrayId(int box_id) {
    int id = 0;
    try {
        std::cout << "boxxxxxxxxxxx" << box_id << std::endl;

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetTrayIdByBoxId(?)"));
        pstmt->setInt(1, box_id); // Set the input parameter

     //   pstmt->execute();
        sql::ResultSet* resultSet = pstmt->executeQuery();
        // Retrieve the value of trayid from the stored procedure result set
     //   std::unique_ptr<sql::Statement> stmt(con->createStatement());
     //   std::unique_ptr<sql::ResultSet> resultSet(stmt->executeQuery("SELECT @trayid"));
        if (resultSet->next()) {
            id = resultSet->getInt("trayid"); // Retrieve the value of trayid from the result set
            std::cout << "trayyyyyyyyy" << id << std::endl;
       }
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    return id;
}


// Add a new method to your Database class to call the stored procedure and retrieve the list of unstored box IDs
std::vector<int> Database::getUnstoredBoxes() {
    std::vector<int> unstoredBoxes; // Vector to store the list of unstored box IDs

    try {

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetUnstoredBoxes()"));

        // Execute the query
        sql::ResultSet* resultSet = pstmt->executeQuery();

        while (resultSet->next()) {
            int boxId = resultSet->getInt("id");
            unstoredBoxes.push_back(boxId);
        }

        // Clean up
        delete resultSet;
        pstmt->clearParameters();

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return unstoredBoxes;
}


std::vector<int> Database::getStoredBoxes() {
    std::vector<int> unstoredBoxes; // Vector to store the list of unstored box IDs

    try {

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL allStoredBoxes()"));

        // Execute the query
        sql::ResultSet* resultSet = pstmt->executeQuery();

        while (resultSet->next()) {
            int boxId = resultSet->getInt("boxid");
            unstoredBoxes.push_back(boxId);
        }

        // Clean up
        delete resultSet;
        pstmt->clearParameters();

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return unstoredBoxes;
}


