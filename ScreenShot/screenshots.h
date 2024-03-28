#ifndef SCREENSHOTS_H
#define SCREENSHOTS_H

#include <QObject>
#include <QDebug>
/**
 * 截屏控件
 */

#include <QMenu>
#include <QCheckBox>
#include <QWidget>
#include <QLineEdit>
#include <QInputMethodEvent>

#include "selectshotarea.h"
#include "drawcontent.h"

class QPushButton;
class ScreenShots : public QWidget
{
    Q_OBJECT

    SelectShotArea sa;
    QPixmap background;
    QPixmap grayBackground;
    QPixmap show_pix;
    DrawContent draw;
    QWidget *buttons;

public:
    explicit ScreenShots(QWidget *parent = 0);
    void initFrame();
    void showRaise();

private slots:
    void on_save_image();

protected:
    void inputMethodEvent(QInputMethodEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *);
    void leaveEvent(QEvent *event) override;

private:
    void showButtons(QRect select_rect);
    void updateMouseShape(QPoint pos);
};
#endif // SCREENSHOTS_H
