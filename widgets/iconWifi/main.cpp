#include "iconWifi.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    iconWifi w;
    w.show();

    return a.exec();
}
