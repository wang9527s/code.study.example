#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>


#include <QSqlDatabase>
#include <QDebug>
#include <QApplication>
#include <QMainWindow>
#include <QTableView>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QTableView>
#include <QSqlTableModel>
#include <QLabel>


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);

private slots:
    void onUpdateData();
    void onSearch(QString content);
    void addRow();

private:
    void removeRow(int row);

    QTableView * view;
    QSqlQueryModel model;
    QLabel * icon;
};
#endif // WIDGET_H
