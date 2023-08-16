#include <QCoreApplication>

#include <QSystemSemaphore>
#include <QDebug>
#include <QThread>
#include <QTime>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSystemSemaphore sem("capture-start", 1, QSystemSemaphore::Open);
    QSystemSemaphore sem_end("capture-end", 0, QSystemSemaphore::Open);
    while(true)
    {
        QThread::sleep(3);
        sem.release(); //只release 不acquire
        sem_end.acquire();
        qInfo() << "capture" << QTime::currentTime().toString("hh:mm:ss.zzz");
    }

    return a.exec();
}
