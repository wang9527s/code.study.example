#include <QCoreApplication>

#include <QObject>

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpServer>

#define MyTcpPort 8886

struct ClientInfo{
    QTcpSocket * socket;
    int id;
    QString name;
};

class TCPServer : public QObject
{
    Q_OBJECT

public:
    TCPServer(QObject *parent = nullptr) {
        _tcpServer = new QTcpServer;

        int ret = _tcpServer->listen(QHostAddress::AnyIPv4,MyTcpPort);
        if(ret==0)
        {
            qDebug()<<"_tcpServer->listen is failied";
            return;
        }
        connect(_tcpServer,&QTcpServer::newConnection,this,&TCPServer::onNewClientConnected);
    }
    ~TCPServer() {
        for (auto cl: clients ) {
            qInfo() << "delte client";
            cl.socket->disconnect();
            cl.socket->abort();
            cl.socket->close();
        }

        if(_tcpServer)
        {
            _tcpServer->close();
            delete _tcpServer;
        }
    }

protected slots:
    void send(){
        for (auto cl: clients ) {
            QString inputText = "hello" + cl.name;
            if (cl.socket->write(inputText.toStdString().c_str()) < 0) {
                qDebug()<<"write to client is failed!" << cl.id << cl.name;
            }
        }
    }
    void onNewClientConnected() {
        if(_tcpServer->hasPendingConnections())
        {
            qInfo() << "recv new connection";
            auto _tcpSocket = _tcpServer->nextPendingConnection();
            if(!_tcpSocket->isValid()) return;
            connect(_tcpSocket,&QTcpSocket::readyRead,this,
                    &TCPServer::onRecvData);
            connect(_tcpSocket,&QTcpSocket::disconnected,this,
                    &TCPServer::onClientDisconnected);

            static int idx =0;
            idx++;

            ClientInfo ifo;
            ifo.id = idx;
            ifo.socket = _tcpSocket;
            clients.append(ifo);
        }
    }
    void onRecvData() {
        QTcpSocket * socket = qobject_cast<QTcpSocket*>(sender());
        if (socket == nullptr)
            return;

        QString recvData = socket->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(recvData.toLocal8Bit().data());
        if (jsonDocument.isNull()) {
            return;
        }

        QJsonObject json = jsonDocument.object();

        QString cli_name = json.value("name").toString();
        for (auto cl: clients ) {
            if (cl.socket = socket) {
                if (cl.name == "")
                    cl.name = cli_name;

                QString msg = "hello " + cli_name + " server recv this msg" + recvData;
                if (cl.socket->write(msg.toStdString().c_str()) < 0) {
                    qDebug()<<"write to client is failed!" << cl.id << cl.name;
                }
                break;
            }
        }
        qDebug()<<"recvData:"<<json;
    }
    void onClientDisconnected(){
        QTcpSocket * socket = qobject_cast<QTcpSocket*>(sender());
        if (socket == nullptr)
            return;

        QString clientIp = socket->peerAddress().toString();
        qInfo() << clientIp << "disconnectd";
    }


private:
    QTcpServer *_tcpServer;
    QList<ClientInfo> clients;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TCPServer server;
    return a.exec();
}
