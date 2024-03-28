#ifndef COMPANY_H
#define COMPANY_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>

#define ServiceName "com.wangbin.daemon"
#define ServicePath "/wangbin"
#define ServiceInterface "com.wangbin.daemon.interface"

Q_DECLARE_METATYPE(QJsonObject)

class DBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", ServiceInterface)

public slots:
    void doLoop(bool start)
    {
        qInfo() << "doLoop" << start;
        static int idx = 0;
        static QTimer *pTimer = new QTimer(this);
        pTimer->setInterval(800);
        connect(pTimer, &QTimer::timeout, this, [=] { emit this->sigProgress(idx++, "hh"); });
        if (!pTimer->isActive())
            pTimer->start();
    };
    QString result()
    {
        return "success";
    }
signals:
    void sigProgress(int totalNum, QString messge);
};

#endif // COMPANY_H
