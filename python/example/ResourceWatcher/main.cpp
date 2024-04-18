
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

#include "widget.h"

int main(int argc, char *argv[])
{
    qputenv("QT_MESSAGE_PATTERN", "%{file}: %{line} - %{message}");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    Widget w;
    w.show();

    return a.exec();
}

