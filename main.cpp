#include "mainwindow.h"
#include "Database.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <cstdlib>
#include <iostream>
#include <array>
#include <string>
#include "Detection2D.h"
#include <QDebug>
#include "pcl_3d.h"
#include <QMetaType>
#include <memory>
#include "Task.h" // Make sure this includes your Task class definition

using namespace std;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<std::shared_ptr<Task>>("std::shared_ptr<Task>");

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
    
}



