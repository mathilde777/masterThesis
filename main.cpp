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
