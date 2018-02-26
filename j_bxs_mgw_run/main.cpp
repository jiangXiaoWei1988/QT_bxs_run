#include "maindialog.h"
#include <QApplication>
#include <QTranslator>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator *t = new QTranslator;
    t->load("transsrc.qm");
    a.installTranslator(t);
    MainDialog w;
    w.show();
    return a.exec();
}
