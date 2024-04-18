#ifndef UPDATEINTER_H
#define UPDATEINTER_H

#include <QObject>
#include <QJsonObject>

#define ServiceName "com.wangbin.daemon"
#define ServicePath "/wangbin"
#define ServiceInterface "com.wangbin.daemon.interface"

#include <QDBusInterface>
#include <QDebug>
#include <QDBusPendingReply>

/*
        TODO 无法监听来自 dbus-send和qdbus等工具发送的dbus信号
*/

class Client : public QObject
{
    Q_OBJECT
public:
    Client(QObject *parent = nullptr)
        : QObject(parent)
        , m_inter(new QDBusInterface(
              ServiceName, ServicePath, ServiceInterface, QDBusConnection::sessionBus()))
    {
        connect(m_inter, SIGNAL(sigProgress(int, QString)), this, SLOT(onProgress(int, QString)));

        getResult();
        doloop();
    }

    QString getResult()
    {
        QDBusPendingReply<QString> reply = m_inter->asyncCall(QLatin1String("result"));
        reply.waitForFinished();
        QString res = reply;
        qInfo() << "getResult: " << res;
        return res;
    }

    void doloop()
    {
        m_inter->asyncCall(QLatin1String("doLoop"), QVariant(true));
    }
public slots:
    void onProgress(int code, QString messge)
    {
        qInfo() << "clicked recv:" << code << messge;
    }

private:
    QDBusInterface *m_inter;
};

#endif // UPDATEINTER_H
