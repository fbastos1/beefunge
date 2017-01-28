#include "beefunge.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    ///todo: automatically open file if given as argument
    ///todo 2: add flag to run as command line (interpreter-only)
    
    QApplication a(argc, argv);
    
    Beefunge w;
    w.show();

    return a.exec();
}
