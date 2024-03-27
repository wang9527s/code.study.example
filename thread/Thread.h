#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QDebug>

class Thread:public QThread
{
    Q_OBJECT

public slots:
    void slot_main()
    {
        qDebug()<<"from thread slot_main:" <<currentThreadId();
    }
protected:
    void run() {
        qDebug()<<"thread thread:"<<currentThreadId();
    }
};

#endif // THREAD_H
