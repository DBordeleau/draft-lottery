#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int fontId = QFontDatabase::addApplicationFont(":/fonts/InterTight-VariableFont_wght.ttf");

    if (fontId != -1)
    {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);

        QFont defaultFont(fontFamily);
        defaultFont.setPointSize(12);
        QApplication::setFont(defaultFont);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
