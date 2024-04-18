#include "dbusservice.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DBusService s;
    // new connection
    QDBusConnection connection = QDBusConnection::sessionBus();
    // registe service
    if (!connection.registerService(ServiceName)) {
        qInfo() << "register service failed" << connection.lastError().message();
        exit(1);
    }

    // registe object
    connection.registerObject(ServicePath, &s, QDBusConnection::ExportAllContents);

    return a.exec();
}
