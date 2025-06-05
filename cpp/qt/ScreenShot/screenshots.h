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
#include <QLabel>
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

    QWidget *buttons;
    OperateType operate_type = SelectArea;

    QString inputText;
    QPoint press_pos;
    QPoint end_pos;
    QRect rt;
    QList<DrawItemBase *> items;

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

private:
    void initButtons();
    void showButtons(QRect select_rect);
    void updateMouseShape(QPoint pos);
    void save_and_exit(QString path, QString name);
};
#endif // SCREENSHOTS_H
