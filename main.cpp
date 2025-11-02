#include <QApplication>
#include "gamewindow.h"
#include <iostream>
#include <QMessageBox>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameWindow w;
    w.show();
    return a.exec();
}