#include "screenshots.h"

#include <QApplication>
#include <QFontDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //    bool ok;
    //    QFontDialog::getFont(&ok, QFont("Arial", 12), nullptr, "选择字体");
    ScreenShots w;
    w.showRaise();
    return a.exec();
}
