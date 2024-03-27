#ifndef INSTANCESIGNAL_H
#define INSTANCESIGNAL_H

#include <QObject>
#include <QDebug>
#include <QThread>


class InstanceSignal : public QObject {
    Q_OBJECT

public:
    static InstanceSignal * instance() {
        static InstanceSignal * pIns = nullptr;
        if (pIns == nullptr)
            pIns = new InstanceSignal;

        return pIns;
    }

signals:
    void sigText(QString text);
};

class MainMsg : public QObject
{

public:
    MainMsg(QObject * parent = nullptr):QObject{parent} {
        qInfo() << "Main thread: " << QThread::currentThreadId();

        connect(InstanceSignal::instance(), &InstanceSignal::sigText,this,
                [](QString text)
        {
            qInfo() << QThread::currentThreadId() << "slot recv msg, "<< text;
        }, Qt::BlockingQueuedConnection);
    }
};

#endif // INSTANCESIGNAL_H
