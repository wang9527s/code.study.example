#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QThread>
#include <QJsonObject>

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    void load(QJsonObject json);
    ~Widget();

signals:
    void update_usage(QJsonObject);

private:
    QTableWidget * table;
    QTextEdit * edit;
    QCheckBox * show_realtime;
    QLabel * title;
    QThread  * thread;
    bool stop_thread = false;
};

#endif // WIDGET_H
