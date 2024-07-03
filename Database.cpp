#include "Database.h"
#include "Task.h"
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
/**
 * @brief Database::addTask
 * @param box_id
 * @param task_type
 * @param tray_id
 *
 * adds a task to database
 */
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
/**
 * @brief Database::newKnownBox
 * @param name
 * @param width
 * @param height
 * @param lenght
 * creates a new known box in the database
 */
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

/**
 * @brief Database::getTasks
 * @param tray_id
 * @return
 * returns all tasks for a tray
 */
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

/**
 * @brief Database::updateBox
 * @param id
 * @param last_x
 * @param last_y
 * @param last_z
 * @param cx
 * @param cy
 * @param cz
 * updates the position of a box in the tray stored in the database
 *
 */
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

/**
 * @brief Database::storeBox
 * @param id
 * @param tray
 *  adds a box to teh tray adn thus the database
 */
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
}


/**
 * @brief Database::checkStoredBoxes
 * @param boxId
 * @return
 * checks if a box is stored
 */
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
/**
 * @brief Database::checkKnownBoxes
 * @param boxId
 * @return
 * check if a box is known
 */

bool Database::checkKnownBoxes(int boxId) {
    bool foundBox = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL check_known_box(?, @found_box)"));
        pstmt->setInt(1, boxId);
        pstmt->execute();


        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

        }

        // Now, retrieve the output parameter value
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT @found_box AS found_box"));

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




/**
 * @brief Database::removeStoredBox
 * @param boxId
 * remove a stored box from the database
 */
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


/**
 * @brief Database::getBoxInfo
 * @param Id
 * @return
 * returns all data about a box
 */
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

/**
 * @brief Database::checkExistingBoxes
 * @param tray_id
 * @param box_id
 * @return
 * checks if there are boxes of sme dimensions in a certain tray
 */
bool Database::checkExistingBoxes(int tray_id, int box_id) {
    bool exists = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL CheckForSameDimensions(?, ?)"));
        pstmt->setInt(1, tray_id);
        pstmt->setInt(2, box_id);

        bool isResult = pstmt->execute();

        if (isResult) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());
            exists = resultSet && resultSet->next();
        }


        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
    return exists;
}


/**
 * @brief Database::removeTaskFromQueue
 * @param taskId
 * remove task once executed
 */

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

/**
 * @brief Database::getTrayId
 * @param box_id
 * @return
 * get tray id from stored box
 */
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

/**
 * @brief Database::getKnownBoxes
 * @return
 * return all known boxes in a tray
 */

std::vector<std::shared_ptr<KnownBox>> Database::getKnownBoxes() {
    std::vector<std::shared_ptr<KnownBox>> knownBoxes = std::vector<std::shared_ptr<KnownBox>>();


    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL allKnownBoxes()"));
        bool isResult = pstmt->execute();

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


/**
 * @brief Database::getStoredBoxes
 * @return
 * returns all stored boxes
 */
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
/**
 * @brief Database::getBoxDimensions
 * @param Id
 * @return
 * returns the box dimesnions
 */

std::tuple<double, double, double> Database::getBoxDimensions(int Id) {
    std::tuple<double, double, double> dimensions;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetBoxDimensions(?)"));
        pstmt->setInt(1, Id);
        bool hasResults = pstmt->execute();

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            if (resultSet && resultSet->next()) {
                double width = resultSet->getDouble("width");
                double height = resultSet->getDouble("height");
                double length = resultSet->getDouble("length");
                dimensions = std::make_tuple(width, height, length);
            }
        }


        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return dimensions;
}

/**
 * @brief Database::getReferences
 * @param position
 * @return
 * gets refernce calulated for each specific position of the tray.
 * the tray can arrive at different heights and this takes reference allows the resystem to take it into account
 */
Eigen::Vector3f Database::getReferences(int position) {
    Eigen::Vector3f refPoint(0.0f, 0.0f, 0.0f); // Initialize to a default value

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL GetReference(?)"));
        pstmt->setInt(1, position);
        bool hasResults = pstmt->execute(); // Execute the stored procedure

        if (hasResults) {
            std::unique_ptr<sql::ResultSet> resultSet(pstmt->getResultSet());

            if (resultSet && resultSet->next()) {
                float x = resultSet->getDouble("referenceX");
                float y = resultSet->getDouble("referenceY");
                float z = resultSet->getDouble("referenceZ");
                refPoint = Eigen::Vector3f(x, y, z);
            }
        }

        // Consume any additional result sets to avoid "Commands out of sync" error
        while (pstmt->getMoreResults()) {
            std::unique_ptr<sql::ResultSet> additionalResults(pstmt->getResultSet());
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return refPoint;
}

/**
 * @brief Database::addReference
 * @param posId
 * @param x
 * @param y
 * @param z
 * once calibrated the tray refences can be upated in the database
 */
void Database::addReference(int posId, float x,float y, float z ) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL addReference(?,?, ?, ?)"));
        pstmt->setInt(1, posId);
        pstmt->setDouble(2, x);
        pstmt->setDouble(3, y);
        pstmt->setDouble(4, z);
        pstmt->execute();

    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}

/**
 * @brief Database::getBox
 * @param tray
 * @param Id
 * @return
 * returns a box and its infromation
 */
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

/**
 * @brief Database::getAllBoxesInTray
 * @param trayId
 * @return
 * get a list of stored boxes in a tray
 */
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

/**
 * @brief Database::addTrainingImage
 * @param box_id
 * @param pic
 * adds a pictrue location to the database
 */
void Database::addTrainingImage(int box_id, std::string pic ) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL addTrainingImage(?, ?)"));
        pstmt->setInt(1, box_id);
        pstmt->setString(2, pic);
        pstmt->execute();
        std::cout << "Training image added " << std::endl;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }
}

