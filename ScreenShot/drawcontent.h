#ifndef DRAWCONTENT_H
#define DRAWCONTENT_H

#include <QString>
#include <QPainter>
#include <QDebug>

/*
 * 对蒙版添加矩形、文字、箭头操作。支持撤销
 */
enum OperateType { SelectArea, DrawText, DrawRect, DrawLineArrow };

class DrawItemBase
{
public:
    QPen pen;
    DrawItemBase()
    {
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::red);
        pen.setWidth(3);
    }
    virtual void drawToPainter(QPainter *p) = 0;
    virtual ~DrawItemBase() {}
};

class DrawItemText : public DrawItemBase
{
public:
    DrawItemText(QString text, QRect rt)
        : text(text)
        , rt(rt)
    {
    }
    QString text;
    QRect rt;
    void drawToPainter(QPainter *p);
};
class DrawItemRect : public DrawItemBase
{
public:
    DrawItemRect(QRect rt)
        : rt(rt)
    {
    }
    QRect rt;
    void drawToPainter(QPainter *p);
};
class DrawItemLineArraw : public DrawItemBase
{
public:
    DrawItemLineArraw(QPoint start, QPoint end)
        : start(start)
        , end(end)
    {
    }
    QPoint start;
    QPoint end;
    void drawToPainter(QPainter *p);
    static void drawLineArraw(QPainter *p, QPointF start, QPointF stop);
};

// class DrawContent
//{
// public:
//     void pop_back();

//    void appendItem(DrawType type);

////    DrawItem cur_item;
//    QPoint press_pos;
//    QList<DrawItemBase*> items;
//};

#endif // DRAWCONTENT_H
