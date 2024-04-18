#include "selectshotarea.h"

void SelectShotArea::mousePressed(QPoint pos)
{
    border = judgeBorder(pos);
    if (border != SelectShotArea::Border_None) {
        type = SelectShotArea::ResizeSelectArea;
        isResizePressed = true;
    }
    else if (isInArea(pos)) {
        type = SelectShotArea::MoveSelectArea;
        isMovePressed = true;
        move_pressed_pos = pos;
    }
    else if (type == SelectShotArea::InitSelectArea) {
        isInitPressed = true;
        init_move = init_pressed = pos;
    }
}

void SelectShotArea::mouseRelease()
{
    isResizePressed = false;
    isInitPressed = false;

    if (isMovePressed) {
        isMovePressed = false;

        init_pressed += move_offset;
        init_move += move_offset;
        move_offset = QPoint(0, 0);
    }
}

void SelectShotArea::mouseMove(QPoint pos)
{
    if (isInitPressed) {
        init_move = pos;
    }
    else if (isMovePressed) {
        move_offset = pos - move_pressed_pos;
    }
    else if (isResizePressed) {
        QPoint tl = rt().topLeft();
        QPoint br = rt().bottomRight();
        if (border == SelectShotArea::Left) {
            tl.setX(pos.rx());
        }
        else if (border == SelectShotArea::Right) {
            br.setX(pos.rx());
        }
        else if (border == SelectShotArea::Top) {
            tl.setY(pos.ry());
        }
        else if (border == SelectShotArea::Bottom) {
            br.setY(pos.ry());
        }

        init_pressed = tl;
        init_move = br + QPoint(1, 1);
    }
}

QRect SelectShotArea::rt()
{
    QPoint p1 = init_pressed;
    QPoint p2 = init_move;

    p1 += move_offset;
    p2 += move_offset;

    int x = qMin(p1.x(), p2.x());
    int y = qMin(p1.y(), p2.y());
    int w = qAbs(p1.x() - p2.x());
    int h = qAbs(p1.y() - p2.y());
    return QRect(x, y, w, h);
}

SelectShotArea::Border SelectShotArea::judgeBorder(QPoint pos)
{
    QRect rect = rt();
    const int t = rect.top();
    const int b = rect.bottom();
    const int l = rect.left();
    const int r = rect.right();

    const int x = pos.x();
    const int y = pos.y();

    const int border_width = 8;
    if (qAbs(x - l) < border_width) {
        if (y > t && y < b)
            return Left;
    }
    if (qAbs(x - r) < border_width) {
        if (y > t && y < b)
            return Right;
    }
    if (qAbs(y - t) < border_width) {
        if (x > l && x < r)
            return Top;
    }
    if (qAbs(y - b) < border_width) {
        if (x > l && x < r)
            return Bottom;
    }

    return Border_None;
}

bool SelectShotArea::isInArea(QPoint pos)
{
    return rt().contains(pos);
}
