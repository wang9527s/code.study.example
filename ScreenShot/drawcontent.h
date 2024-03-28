#ifndef DRAWCONTENT_H
#define DRAWCONTENT_H

#include <QString>
#include <QPainter>
#include <QDebug>

/*
 * 对蒙版添加矩形、文字操作。支持撤销
 */
enum DrawType {
    None,
    Text,
    Rect,
};

struct DrawItem {
    DrawType type = None;
    QRect rt;
    QString text;

    // 无效的
    bool notValid()
    {
        if (type == DrawType::None)
            return true;
        if (rt.width() <= 0 || rt.height() <= 0)
            return true;
        //        if (type == DrawType::Text && text.isNull())
        //            return true;

        return false;
    }
    void drawToPainter(QPainter *p)
    {
        if (type == DrawType::None)
            return;
        if (rt.width() <= 0 || rt.height() <= 0)
            return;

        QPen pen;
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::red);
        pen.setWidth(3);
        p->setPen(pen);
        if (type == DrawType::Rect) {
            p->drawRect(rt);
        }
        else if (type == DrawType::Text) {
            QStringList lines = text.split("\n");
            int y = rt.y();
            QFontMetrics fm(p->font());
            for (const QString &line : lines) {
                QRect boundingRect = fm.boundingRect(
                    rt.x(), y, rt.width(), 0, Qt::AlignLeft | Qt::TextWordWrap, line);
                p->drawText(boundingRect, Qt::AlignLeft | Qt::TextWordWrap, line);
                y += boundingRect.height(); // 递增y以容纳下一行的绘制
                y += 12;                    // 间隔大点
            }
        }
    }
};

class DrawContent
{
public:
    bool enable()
    {
        return cur_item.type != None;
    }
    void pop_back()
    {
        if (cur_item.type == DrawType::Text && cur_item.text != "") {
            cur_item.rt = QRect();
            cur_item.text = "";
            return;
        }

        if (items.size() >= 1) {
            items.pop_back();
        }
    }
    void appendItem(DrawType type)
    {
        bool ignore = type != cur_item.type || cur_item.rt.width() < 1 || cur_item.rt.height() < 1;
        //        if (cur_item.type == DrawType::Text && cur_item.text == "")
        //            ignore = true;
        if (ignore) {
            qInfo() << "append ignore";
            return;
        }

        qInfo() << "append";

        items.append(cur_item);
        cur_item.rt = QRect();
        cur_item.text = "";
    }

    DrawItem cur_item;
    QPoint press_pos;
    QList<DrawItem> items;
};

#endif // DRAWCONTENT_H
