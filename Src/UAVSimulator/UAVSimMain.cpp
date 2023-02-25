#include "UAVSimMainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UAVSimMainWindow w(nullptr);
    w.show();

    return a.exec();
}
