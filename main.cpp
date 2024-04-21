#include "mainwindow.h"
#include "database.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <cstdlib>
#include <iostream>
#include <array>
#include <string>
#include "detection2D.h"
#include <QDebug>
#include "/home/user/Documents/Thesis/library/lib/include/pcl_3d.h"

using namespace std;
//#include <QtSqlDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "simulation_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();






    return a.exec();
    /**
    // Open a pipe to capture the output of the curl command
    FILE *pipe = popen("curl -X POST http://localhost:5001/predict -H \"Content-Type: application/json\" -d \"{\\\"img_path\\\": \\\"test4.jpeg\\\"}\" 2>&1", "r");
    if (!pipe) {
        std::cerr << "Failed to open pipe to curl command." << std::endl;
        return 1;
    }

    // Read the output of the curl command from the pipe
    std::array<char, 128> buffer;
    std::string result;
    while (!feof(pipe)) {
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    }

    // Close the pipe
    pclose(pipe);

    // Output the result
    std::cout << "Response from curl command:\n" << result << std::endl;

    **/

}

int test()
{
    cout << "Hello World!" << endl;

    //PLY Files
    string filePathBoxes = "/home/suleyman/Desktop/MasterThesis/ModelsV3/kinect_middle.ply";
    string filePathEmpty = "/home/suleyman/Desktop/MasterThesis/ModelsV3/empty_tray.ply";
    string filePathBrownBox = "/home/suleyman/Desktop/MasterThesis/ModelsV3/3box_center_big_gap.ply";
    string filePathBrownBox2 = "/home/suleyman/Desktop/MasterThesis/ModelsV3/kinect_top.ply";
    string fileboxsmallgap = "/home/suleyman/Desktop/MasterThesis/ModelsV3/3box_center_v2_low_gap.ply";
    string filetwoboxtogether = "/home/suleyman/Desktop/MasterThesis/ModelsV3/2box_together_edge_v2.ply";
    string fileboxangled = "/home/suleyman/Desktop/MasterThesis/ModelsV3/whitebox_angled.ply";

    //initiate pcl_3d
    PCL_3D pcl3d;
    auto refPoint = pcl3d.calibrateTray(filePathEmpty, 690);

    //put ref point manually for now : Reference point: 457 352.699 699.949"
    //auto refPoint = Eigen::Vector3f(457, 352.699, 699.949);

    cout << "Reference point: " << refPoint.x() << " " << refPoint.y() << " " << refPoint.z() << endl;



    auto location = pcl3d.findBoundingBox(fileboxangled , filePathEmpty, refPoint, Eigen::Vector3f(0, 0, 0));

    //location is type of clusterInfo ==> struct ClusterInfo { Eigen::Vector4f centroid, Eigen::Vector3f dimensions; Eigen::Quaternionf orientation; int clusterId;
    for (auto loc : location)
    {
        cout << "Cluster ID: " << loc.clusterId << endl;
        cout << "Centroid: " << loc.centroid.x() << " " << loc.centroid.y() << " " << loc.centroid.z() << endl;
        cout << "Dimensions: " << loc.dimensions.x() << " " << loc.dimensions.y() << " " << loc.dimensions.z() << endl;
        cout << "Orientation: " << loc.orientation.x() << " " << loc.orientation.y() << " " << loc.orientation.z() << " " << loc.orientation.w() << endl;
    }


    return 0;
}


//todo
/**
- COORDINATE SYSTEM
- ADDING TAKS WHILE DOCKED
- 2D LOGIC
- 3D LOGIC



**/
