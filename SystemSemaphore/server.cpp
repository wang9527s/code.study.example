#include <QCoreApplication>

#include <QSystemSemaphore>
#include <QDebug>
#include <QThread>
#include <QTime>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSystemSemaphore sem("capture-start", 0, QSystemSemaphore::Create);
    QSystemSemaphore sem_end("capture-end", 1, QSystemSemaphore::Create);
    int i;
    qInfo() << "start";
    while(true)
    {
        sem.acquire(); //只require 不release
        qInfo() << "capture start";
        QThread::msleep(300);
        /* 调用相机接口 */
        sem_end.release();
        qInfo() << "capture end" << QTime::currentTime().toString("hh:mm:ss.zzz");
    }

    return a.exec();
}
