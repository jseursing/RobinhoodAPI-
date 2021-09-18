#include "RHAssets.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RHAssets w;
    w.show();
    return a.exec();
}
