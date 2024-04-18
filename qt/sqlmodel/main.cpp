#include <QApplication>
#include <QMainWindow>
#include <QTableView>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Widget w;
    w.show();

    return app.exec();
}
