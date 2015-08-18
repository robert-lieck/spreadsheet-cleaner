#include "SpreadsheetCleaner.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpreadsheetCleaner w;
    w.show();

    return a.exec();
}
