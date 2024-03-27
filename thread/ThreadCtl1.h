#ifndef THREADCTL1_H
#define THREADCTL1_H

#include <QThread>
#include <QDebug>
#include <QTimer>

class Worker :public QObject {
    Q_OBJECT
public:
    Worker():QObject(){
        initTimer();
    }
    ~Worker(){qInfo() << id <<__func__;}
    void initTimer() {
        if (pTimer!=nullptr)
            return;
        pTimer = new QTimer(this);
        connect(pTimer, &QTimer::timeout, this, [=]{
            qInfo() << id << "working";
        });
    }

    int id;
public slots:
    void onWork(int msec) {
        initTimer();
        qInfo() << u8"开始工作" << id;

        if (pTimer->isActive() == false) {
            pTimer->start(msec);
        }
    }
    void onStart() {
        qInfo() <<"start thread" << QThread::currentThreadId() << id;
    }
    void recvMsg(QString msg) {
        qInfo() << id << msg;
    }

private:
    QTimer * pTimer = nullptr;
};

class ThreadCtl1 :public QThread {
    Q_OBJECT
public:
    ThreadCtl1(){
        for (int i =0; i < 10; i++) {
            Worker* work = new Worker;
            work->id = i;
            QThread* thread = new QThread;
            threads.append(thread);

            work->moveToThread(thread);
            connect(thread,&QThread::finished,work,&Worker::deleteLater);
            connect(thread,&QThread::finished,thread,&QThread::deleteLater);

            // 工作逻辑
            connect(thread,&QThread::started,work,&Worker::onStart);
            connect(this,&ThreadCtl1::sigSendmsg,work,&Worker::recvMsg);
            connect(this,&ThreadCtl1::startWork,work,&Worker::onWork);
        }
    }

    ~ThreadCtl1(){
        deleteThreads();

    }
    void deleteThreads() {
        for (auto p: threads){
            if (p->isFinished() == false) {
                p->quit();
                p->wait();
            }

            threads.removeOne(p);
        }
    }
    void startThread() {
        for (auto p: threads){
            p->start();
        }
    }


signals:
    void sigSendmsg(QString msg);
    void startWork(int msec);

private:
    QList<QThread*> threads;
};

#endif // THREADCTL1_H
