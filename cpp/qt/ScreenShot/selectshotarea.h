#ifndef SELECTSHOTAREA_H
#define SELECTSHOTAREA_H

#include <QPoint>
#include <QPixmap>

/*
 * 选择截屏的区域
 */

class SelectShotArea
{
public:
    void mousePressed(QPoint pos);
    void mouseRelease();
    void mouseMove(QPoint pos);

public:
    QRect rt();

    bool isInitPressed = false;
    bool isResizePressed = false;
    bool isMovePressed = false;
    QPoint init_pressed;
    QPoint init_move;

    QPoint move_pressed_pos;
    QPoint move_offset = QPoint(0, 0);

    enum Type {
        // 对截图框的操作
        InitSelectArea,   // 初始化截图框
        ResizeSelectArea, // 调整截图框的大小
        MoveSelectArea,   // 移动截图框
    };
    Type type = InitSelectArea;

    enum Border { Border_None = 0, Left, Top, Right, Bottom };
    Border border;
    Border judgeBorder(QPoint pos);

    bool isInArea(QPoint pos);
};

#endif // SELECTSHOTAREA_H
