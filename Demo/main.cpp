#include "MainWindow.h"
#include <QApplication>

void regABColorDelegates();
void regIntListDelegates();
void regLayerDelegates();
void regPenWidthDelegates();
void regFreqDelegates();
void regArrayDelegate();

int main(int argc, char *argv[])
{
    regABColorDelegates();
    regIntListDelegates();
    regLayerDelegates();
    regPenWidthDelegates();
    regFreqDelegates();
    regArrayDelegate();

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
