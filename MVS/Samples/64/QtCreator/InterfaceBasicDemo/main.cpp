#include "interfacebasicdemo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    InterfaceBasicDemo w;
    w.show();

    return a.exec();
}
