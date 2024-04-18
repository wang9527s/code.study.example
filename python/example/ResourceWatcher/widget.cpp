#include "widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QHeaderView>
#include <QtConcurrent>


#include "tool.h"

Widget::Widget(QWidget *parent)
    : QWidget{parent}
{
    show_realtime = new QCheckBox(u8"实时更新");
    show_realtime->setCheckState(Qt::Checked);
    title = new QLabel;

    table = new QTableWidget;
    table->setColumnCount(3);
    QStringList title_s = QStringList() << "pid" << u8"进程名"
                                        << u8"内存"   ;
    table->setHorizontalHeaderLabels(title_s);
    table->verticalScrollBar()->setVisible(false);

    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, [=](int logicalIndex) {
        table->sortItems(logicalIndex, Qt::DescendingOrder);
    });

    edit = new QTextEdit;
    edit->setReadOnly(true);
    edit->setFixedHeight(180);

    QVBoxLayout * playout = new QVBoxLayout(this);
    playout->addWidget(show_realtime);
    playout->addWidget(title);
    playout->addWidget(edit);
    playout->addWidget(table);
    setFixedSize(320,690);

    setLayout(playout);

// 不能发送信号
//    QtConcurrent::run([=]{
//        while (1) {
//            PR_from_py e;
//        }
//    });

    thread = new QThread;
    connect(thread, &QThread::started, [&](){
        auto data_pathname = []()->QString{
            QString app_dir = QCoreApplication::applicationDirPath();
            QString date = QDate::currentDate().toString("yyyyMMdd");
            QString path = QString("%1/monitor_data/%2").arg(app_dir).arg(date);

            QString name = QTime::currentTime().toString("hh-mm-ss.zzz") + ".json";

            QDir dir;
            dir.mkpath(path);

            return QDir(path).absoluteFilePath(name);
        };

        QProcess process;
        QString app_dir = QCoreApplication::applicationDirPath();
        QString cmd = QString("%1/monitor_resource.exe").arg(app_dir);

        int idx = 0;
        while (1) {
            if (stop_thread)
                break;
            qInfo()  << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                     << cmd;
            // 获取并保存数据
            QString pathname = data_pathname();
            process.start(cmd);
            process.waitForFinished();
            QJsonObject js = Tool::string2js(QString(process.readAll()).remove("\r\n"));
//            qInfo() << "get js:" << js;
            if (idx % 10 == 1) {
                Tool::saveJosnToFile(pathname, js);
            }

            if (show_realtime->isChecked()) {
                js.insert("time", QFileInfo(pathname).baseName().replace('-', ':'));
                emit this->update_usage(js);
            }
            idx++;
        }
    });
    thread->start();

    connect(this, &Widget::update_usage, this, &Widget::load);
}

void Widget::load(QJsonObject json)
{
    title->setText(json.value("time").toString());
    table->setRowCount(0);
    table->clearContents();

    QJsonObject process = json.value("process").toObject();
    for (auto key: process.keys()) {
        QJsonObject e = process.value(key).toObject();
        QString id = key;
        QString name  = e.value("name").toString();
        QString memory_util  = e.value("memory_util").toString();

        table->insertRow(0);
        table->setItem(0,0,new QTableWidgetItem(id));
        table->setItem(0,1,new QTableWidgetItem(name));
        table->setItem(0,2,new QTableWidgetItem(memory_util));
    }

    QJsonObject utilization = json.value("utilization_rate").toObject();
    QJsonObject psutil = utilization.value("psutil").toObject();
    QString msg = QString(u8"内存：%2\nCPU：%3\n")
            .arg(psutil.value("memory").toString())
            .arg(psutil.value("cpu").toString())
            ;
    msg += "\nGPU:\n";
    for (auto e :json.value("gpu_info").toArray()) {
        QJsonObject gpu_json = e.toObject();
        msg += QString(u8"温度: %1, 使用率：%2，内存：%3\n")
                .arg(gpu_json.value("gpu_temp").toString())
                .arg(gpu_json.value("gpu_utilization").toString())
                .arg(gpu_json.value("gpu_memory").toString());

        for(auto e : gpu_json.value("process_use_gpu_nvidia").toArray()){
            QJsonObject gpu_process = e.toObject();
            msg += QString("%1: %2\n")
                    .arg(gpu_process.value("pid").toString().toInt())
                    .arg(gpu_process.value(" process_name").toString())
                    ;

        }
    }
    edit->setText(msg);
}

Widget::~Widget()
{
    stop_thread = true;
    thread->quit();
    thread->wait();
    thread->deleteLater();
    qInfo() << "exit";
}
