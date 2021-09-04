#include "tradereminder.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    tradeReminder w;

    w.resize(QSize(1600,1200));
    w.show();

    return a.exec();
}
