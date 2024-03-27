
#include <QApplication>
#include "threadtool.h"
#include "InstanceSignal.h"
#include "ThreadCtl1.h"
#include "thread.h"
#include <iostream>

#include <QtConcurrent>
#include <QFuture>

void func() {
    qInfo() << __func__;
}

int my_func2(int value)
{
  qDebug()  << QThread::currentThreadId() << value;
  QThread::msleep(500 * value);
  return value - 1;
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    InstanceSignal::instance();
    MainMsg msg;

    QFuture<QString> res = QtConcurrent::run([]()->QString{
        QThread::msleep(1000);
        // 费时操作（线程中）
        qInfo() << QThread::currentThreadId() << "son thread emit";
        emit InstanceSignal::instance()->sigText("son thread");
        qInfo() << QThread::currentThreadId() << "son thread emit over";
        return "over";
    });

//    qInfo() << QThread::currentThreadId() << "main thread emit";
//    emit InstanceSignal::instance()->sigText("main thread");
//    qInfo() << QThread::currentThreadId() << "main thread emit over";






//    ThreadTool::test1();

//    ThreadCtl1 ctl;
//    ctl.startThread();
//    emit ctl.sigSendmsg("hello");
//    emit ctl.startWork(1800);
//    QTimer::singleShot(2000, [&]{
//        ctl.deleteThreads();
//        qInfo() << u8"释放资源";
//    });

//        Thread t;
//        t.start();

//        QObject::connect(InstanceSignal::instance(), &InstanceSignal::sigText
//                         ,&t, &Thread::slot_main, Qt::BlockingQueuedConnection);
//        QTimer::singleShot(1000,[]{
//        emit InstanceSignal::instance()->sigText("");
//            });


    qInfo() <<u8"开始事件循环";
    return a.exec();
}
