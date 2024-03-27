#ifndef THREADTOOL_H
#define THREADTOOL_H

#include <QRunnable>
#include <QDebug>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include "InstanceSignal.h"

/*
    可以使用两种方法和外界通信：

    1. 发送一个单例类的信号
    2. 多继承，继承自QRunnable和QObject
*/
class HelloWorldTask : public QRunnable
{
public:
    void run()
    {
        QString msg = QString("taskid: %1, threadaddr: %2").arg(m_id).arg(2);

        int m_dataMem[256*1000];
        qInfo() << "start taskid," << m_id;
        for (int nCount = 0; nCount < 2; nCount++)
        {
            qDebug() << __FUNCTION__ << "taskid:" <<m_id
                     << ", work count:" << nCount
                     << ", thrad addr:"<< QThread::currentThreadId()
                     ;
            QThread::msleep(1000);
        }


        emit InstanceSignal::instance()->sigText(msg);
    }

    int m_id;
    HelloWorldTask(int id) :m_id(id) {};
    ~HelloWorldTask() { qDebug() << __FUNCTION__ << m_id; }
};

class ThreadTool {
public:
    static void test1(){
        QThreadPool      threadpool;
        threadpool.setMaxThreadCount(3);         // 线程池中最大的线程数
        for (int nNum = 0; nNum < 6; nNum++)
        {
            HelloWorldTask     *task=new HelloWorldTask(nNum);    // 循环构建可在线程池中运行的任务
            threadpool.start(task);      //线程池分配一个线程运行该任务
        }
        qInfo() << "task all started";
    }
};

#endif // THREADTOOL_H
