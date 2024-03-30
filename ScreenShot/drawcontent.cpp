#include "drawcontent.h"

#include <QtMath>

void DrawItemText::drawToPainter(QPainter *p)
{
    QStringList lines = text.split("\n");
    int y = rt.y();
    QFontMetrics fm(p->font());
    for (const QString &line : lines) {
        QRect boundingRect =
            fm.boundingRect(rt.x(), y, rt.width(), 0, Qt::AlignLeft | Qt::TextWordWrap, line);
        p->drawText(boundingRect, Qt::AlignLeft | Qt::TextWordWrap, line);
        y += boundingRect.height(); // 递增y以容纳下一行的绘制
        y += 12;                    // 间隔大点
    }
}

void DrawItemRect::drawToPainter(QPainter *p)
{
    p->drawRect(rt);
}

void DrawItemLineArraw::drawToPainter(QPainter *p)
{
    drawLineArraw(p, start, end);
}

void DrawItemLineArraw::drawLineArraw(QPainter *p, QPointF start, QPointF stop)
{
    float theta1 = std::atan2f(stop.y() - start.y(), stop.x() - start.x());
    stop = QPointF(stop.x() - 10 * std::cosf(theta1), stop.y() - 10 * std::sinf(theta1));

    QLineF line(start, stop);
    p->drawLine(line);

    QLineF v = line.unitVector();
    if (!v.isNull()) {
        v.setLength(10);
        v.translate(QPointF(line.dx(), line.dy()));
        QLineF n = v.normalVector();
        n.setLength(n.length() * 0.5);
        QLineF n2 = n.normalVector().normalVector();
        QPointF p1 = v.p2();
        QPointF p2 = n.p2();
        QPointF p3 = n2.p2();

        if (p1.isNull() == false && p2.isNull() == false) {
            QLineF lineA(p1, p2);
            if (lineA.length() > 4) {
                p->drawLine(lineA);
            }
        }
        if (p2.isNull() == false && p3.isNull() == false) {
            QLineF lineB(p2, p3);
            if (lineB.length() > 4) {
                p->drawLine(lineB);
            }
        }
        if (p3.isNull() == false && p1.isNull() == false) {
            QLineF lineC(p3, p1);
            if (lineC.length() > 4) {
                p->drawLine(lineC);
            }
        }
    }
}
