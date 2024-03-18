#include "widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSqlRecord>
#include <QPushButton>
#include <QFile>
#include <QLineEdit>

QStringList names = QStringList()
        << u8"大嘴" << u8"葵花" << "Tony" << "Sony" << "Joy"
        << u8"爱丽丝" << u8"晴律师" << u8"喜马拉雅" << u8"刘快乐"
        << u8"百里" << u8"里斯" << u8"李昂" << u8"小郭" << u8"我在";

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QPushButton * addbtn = new QPushButton(u8"添加记录");
    QPushButton * delbtn = new QPushButton(u8"删除记录(delete)");
    QPushButton * updateBtn = new QPushButton(u8"更新记录");
    QLineEdit * search = new QLineEdit;
    search->setPlaceholderText(u8"请输入查询");
    icon = new QLabel;
    view = new QTableView;
    QHBoxLayout * btnpl = new QHBoxLayout;
    btnpl->addWidget(addbtn);
    btnpl->addWidget(delbtn);
    btnpl->addWidget(updateBtn);
    btnpl->addWidget(search);
    btnpl->addStretch();
    btnpl->addWidget(icon);
    QVBoxLayout * pl = new QVBoxLayout(this);
    pl->addLayout(btnpl);
    pl->addWidget(view);
    connect(view, &QTableView::clicked, [=](const QModelIndex &index){
        QSqlRecord curRec=model.record(index.row());

        QVariant va= curRec.value("image");
        if (va.isValid()) {
            QByteArray data=va.toByteArray();
            QPixmap pic;
            pic.loadFromData(data);
            icon->setPixmap(pic.scaledToWidth(100));
        }
    });
    connect(addbtn, &QPushButton::clicked,this,[=]{addRow();});
    connect(delbtn, &QPushButton::clicked,this,[=]{
        int row = view->currentIndex().row();
        removeRow(row);
    });
    connect(updateBtn, &QPushButton::clicked,this, &Widget::onUpdateData);
    connect(search, &QLineEdit::textChanged, this, &Widget::onSearch);


    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");
    if (!db.open()) {
        qDebug() << "open db failed" << db.lastError().text();
    }

    // 创建表格
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT,"
                    " age INTEGER, date DATE, image BLOB)")) {
        qDebug() << "create table failed" << query.lastError().text();
    }

    // 查询数据
    model.setQuery("SELECT * FROM person");
    model.setHeaderData(0, Qt::Horizontal, "ID");
    model.setHeaderData(1, Qt::Horizontal, "姓名");
    model.setHeaderData(2, Qt::Horizontal, "年龄");
    model.setHeaderData(3, Qt::Horizontal, "日期");
    model.setHeaderData(4, Qt::Horizontal, "照片");

    int init_count = 5 - model.rowCount();
    while( init_count-- > 0 && true) {
        addRow();
    }

    // 创建主窗口和表格视图
    view->setModel(&model);
    resize(1600, 900);
}

void Widget::onUpdateData()
{
    int id = model.record(view->currentIndex().row()).value("id").toInt();

    QSqlQuery query;
    QString name = names[qrand() % 10];
    query.prepare("update person set name = :name, age = :age, date = :date, image = :image"
                  " where ID = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":age", qrand() % 55);
    query.bindValue(":date", QDateTime::currentDateTime());

    QByteArray data;
    QFile* file=new QFile(":/icon/img/01.png");
    file->open(QIODevice::ReadOnly);
    data = file->readAll();
    file->close();
    query.bindValue(":image", data);

    if (!query.exec()) {
        qDebug() << u8"update failed" << query.lastError().text();
    }

    // 同步数据到界面
    model.setQuery("SELECT * FROM person");
}

void Widget::onSearch(QString content)
{
    const QString text = content;
    if (text == "") {
        model.setQuery("SELECT * FROM person");
    }
    else {
        QString sql;
        // 输入id的全称，或者名字的全称
        sql = QString("select * from person Where id Like '%1' OR name Like '%1'").arg(text);
        // 不需要输入全称
        sql = QString("select * from person Where id Like '%%1%' OR name Like '%%1%'").arg(text);
        // 略  既然有 OR 语句，那么也可以有 AND语句
        qInfo() << sql;
        model.setQuery(sql);
    }
}

void Widget::addRow() {
    QSqlQuery query;
    query.prepare("INSERT INTO person (name, age, date, image) VALUES (?, ?, ?, ?)");
    query.addBindValue(names[qrand() % 10]);
    query.addBindValue(qrand() % 55);
    query.addBindValue(QDateTime::currentDateTime());

    QByteArray data;
    QFile* file=new QFile(":/icon/img/00.JPG");
    file->open(QIODevice::ReadOnly);
    data = file->readAll();
    file->close();

    query.addBindValue(data);

    if (!query.exec()) {
        qDebug() << u8"插入数据失败：" << query.lastError().text();
    }

    // 同步数据到界面
    model.setQuery("SELECT * FROM person");
}
void Widget::removeRow(int row)
{
    QSqlRecord  curRec = model.record(row);
    if (curRec.isEmpty())
        return;

    int id=curRec.value("id").toInt();
    QSqlQuery query;
    query.prepare("delete  from person where ID = :ID");
    query.bindValue(":ID", id);

    if (!query.exec()) {
        qDebug() << u8"删除数据失败：" << query.lastError().text() << ", id:" << id;
    }

    // 同步数据到界面
    model.setQuery("SELECT * FROM person");
}
