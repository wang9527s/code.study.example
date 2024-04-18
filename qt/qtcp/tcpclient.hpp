#include <QCoreApplication>

#include <QDebug>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>

#include <QTcpSocket>

#define MyTcpPort 8886

class TCPClient : public QObject
{
    Q_OBJECT

public:
    TCPClient(QObject *parent = nullptr) {
        _tcpClient= new QTcpSocket(this);
        _tcpClient->abort();
        _tcpClient->connectToHost("127.0.0.1", MyTcpPort);
        if(!_tcpClient->waitForConnected(2000))
        {
            qDebug()<<"connect is failed!";
            return;
        }
        qDebug()<<"connect is successful";
        connect(_tcpClient,&QTcpSocket::readyRead,this,&TCPClient::onRecvData);
    }
    ~TCPClient(){
        _tcpClient->close();
    }

public slots:
    void send() {
        if(!_tcpClient) return;

        static QString name = QString("0x%1").arg(quintptr(QThread::currentThreadId()),0,16);
        static int msgid = 0;
        msgid++;
        QJsonObject json;
        json.insert("name", name);
        json.insert("msgid", msgid);
        json.insert("msg", "hello");

        int ret = _tcpClient->write(QString(QJsonDocument(json).toJson()).toStdString().c_str());

        qDebug()<<"send:" << json;
        if(ret<0)
        {
            qDebug()<<"send data is failed";
        }
    }
    void onRecvData() {
        if(!_tcpClient) return;

        QString recvData= _tcpClient->readAll();

        qDebug()<<"recvData:"<<recvData;
    }

private:
    QTcpSocket *_tcpClient;
};


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TCPClient cli;
    cli.send();
    return a.exec();
}
