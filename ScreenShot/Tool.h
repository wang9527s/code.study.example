#ifndef TOOL_H
#define TOOL_H

#include <QApplication>
#include <QPixmap>
#include <QRect>
#include <QScreen>
#include <QPainter>
#include <QClipboard>

class Tool
{
public:
    // 抓取桌面
    static QPixmap mergeGrabWindow()
    {
        QRect rt(0, 0, 0, 0);
        for (auto pScreen : QGuiApplication::screens()) {
            rt = rt.united(pScreen->geometry());
        }

        QPixmap pix(rt.size());
        QPainter painter(&pix);
        for (auto p : QGuiApplication::screens()) {
            QRect rect = p->geometry();
            auto pixmap = p->grabWindow(0, 0, 0, rect.width(), rect.height());
            painter.drawPixmap(rect.topLeft(), pixmap);
        }

        return pix;
    }

    // 返回两个顶点组成的矩形
    static QRect rt(QPoint p1, QPoint p2)
    {
        int x = qMin(p1.x(), p2.x());
        int y = qMin(p1.y(), p2.y());
        int w = qAbs(p1.x() - p2.x());
        int h = qAbs(p1.y() - p2.y());
        return QRect(x, y, w, h);
    };

    static void copyPixmapToClipboard(QPixmap pixmap)
    {
        QImage image = pixmap.toImage();
        // 获取剪贴板对象
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setImage(image);
    }
};

#endif // TOOL_H
