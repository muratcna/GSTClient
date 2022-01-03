#include <QApplication>

#include "bwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    BWidget w;
    w.show();

    return a.exec();
}
