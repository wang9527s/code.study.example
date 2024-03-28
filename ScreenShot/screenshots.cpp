#include "ScreenShots.h"

#include "qapplication.h"
#include "qdatetime.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qfiledialog.h"
#include "qmutex.h"
#include "qpainter.h"
#include "qstringlist.h"
#include "Tool.h"
#include <QDebug>
#include <QPushButton>
#include <QStandardPaths>
#include "qscreen.h"
#include <QShortcut>
#include <QHBoxLayout>
#include <QTimer>
#include <QDesktopServices>
#include <QButtonGroup>

ScreenShots::ScreenShots(QWidget *parent)
    : QWidget(parent)
{
    initFrame();

    QPushButton *pOk = new QPushButton(u8"确定", this);
    QPushButton *pCancel = new QPushButton(u8"取消", this);

    QCheckBox *draw_text = new QCheckBox(u8"文字", this);
    QCheckBox *draw_rect = new QCheckBox(u8"矩形框", this);
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(draw_text, DrawType::Text);
    buttonGroup->addButton(draw_rect, DrawType::Rect);
    QPushButton *draw_back = new QPushButton(u8"撤销", this);

    setStyleSheet("QPushButton{font: 25px;}"
                  ""
                  "QLineEdit{background:transparent;border: 2px solid #0078D7;}");

    connect(pOk, &QPushButton::clicked, this, &ScreenShots::on_save_image);
    connect(pCancel, &QPushButton::clicked, this, [=] { qApp->quit(); });
    connect(draw_back, &QPushButton::clicked, this, [=] {
        draw.pop_back();
        update();
    });

    setFocusPolicy(Qt::StrongFocus);
    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [&](int id) {
        if (DrawType::Text == draw.cur_item.type) {
            draw.appendItem(DrawType::Text);
        }
        draw.cur_item.type = DrawType(id);
    });

    setAttribute(Qt::WA_InputMethodEnabled, true);
    setMouseTracking(true);

    buttons = new QWidget(this);
    buttons->setVisible(false);
    pOk->setFixedSize(80, 40);
    pCancel->setFixedSize(80, 40);
    buttons->setFixedSize(500, 40);
    QHBoxLayout *buttons_pl = new QHBoxLayout(buttons);
    buttons_pl->setMargin(0);
    buttons_pl->addStretch();
    buttons_pl->addWidget(draw_back);
    buttons_pl->addWidget(draw_rect);
    buttons_pl->addWidget(draw_text);
    buttons_pl->addSpacing(20);
    buttons_pl->addWidget(pOk);
    buttons_pl->addWidget(pCancel);
}

void ScreenShots::initFrame()
{
    // 将整个屏幕的截图保存到 mPix_FullScreen 中
    background = Tool::mergeGrabWindow();

    bool debug = false;
    if (debug) {
        background.fill(Qt::transparent);
        setWindowOpacity(0.66);
        return;
    }

    grayBackground = background.copy();
    QPainter p(&grayBackground);
    QPixmap pixmap(background.size());
    pixmap.fill(QColor(160, 160, 160, 200));
    p.drawPixmap(0, 0, pixmap);
}

void ScreenShots::showRaise()
{
    setAttribute(Qt::WA_StaticContents); // 设置内容静态，避免不必要的重绘
    // Qt::WindowTransparentForInput
    // 窗口不接收输入事件，所以用户不能与其进行交互。
    // Qt::X11BypassWindowManagerHint
    setWindowFlags(Qt::SplashScreen // 不显示任务栏图标，Qt::Tool 会导致失去键盘焦点
                   | Qt::FramelessWindowHint    // 无标题栏
                   | Qt::WindowStaysOnTopHint); // 置顶
    resize(background.size());
    setVisible(true);
    setFocus();
}

void ScreenShots::on_save_image()
{
    QString pathname = QString("%1/%2.png")
                           .arg(QCoreApplication::applicationDirPath())
                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd-hh-mm-ss"));
    QPixmap pix = show_pix.copy(sa.rt());
    pix.save(pathname);
    QDesktopServices::openUrl(pathname);

    qApp->quit();
}

void ScreenShots::inputMethodEvent(QInputMethodEvent *event)
{
    QString txt = event->commitString();

    if (!txt.isNull()) {
        draw.cur_item.text += txt;
        update();
        qInfo() << draw.cur_item.text;
    }
    QWidget::inputMethodEvent(event);
}

void ScreenShots::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        qApp->quit();
    }
    else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        draw.cur_item.text += '\n';
        qInfo() << "enter";
    }
    else if (event->key() == Qt::Key_Back) {
        draw.cur_item.text.remove(draw.cur_item.text.length() - 1, 1);
    }
    return QWidget::keyPressEvent(event);
}

bool ScreenShots::event(QEvent *evt)
{
    QMouseEvent *e = static_cast<QMouseEvent *>(evt);
    if (e == nullptr)
        return QWidget::event(evt);

    QPoint pos = e->pos();
    static bool isPress = false;
    if (e->type() == QEvent::MouseButtonPress) {
        isPress = true;
        if (draw.enable()) {
            draw.press_pos = pos;
            draw.appendItem(DrawType::Text);

            // 给TextItem类型 一个初始值
            draw.cur_item.rt = QRect(pos, QSize(200, 150));
        }
        else {
            sa.mousePressed(pos);
        }
    }
    if (e->type() == QEvent::MouseButtonRelease) {
        isPress = false;
        if (draw.enable()) {
            draw.appendItem(DrawType::Rect);
        }
        sa.mouseRelease();
        setCursor(Qt::ArrowCursor);
    }
    if (e->type() == QEvent::MouseMove) {
        if (isPress) {
            if (draw.enable()) {
                draw.cur_item.rt = Tool::rt(draw.press_pos, pos);
            }
            else {
                // 移动截图区域和操作按钮
                sa.mouseMove(pos);
                showButtons(sa.rt());
            }
        }
        else {
            updateMouseShape(pos);
        }
        update();
    }

    return QWidget::event(evt);
}

void ScreenShots::paintEvent(QPaintEvent *e)
{
    show_pix = grayBackground.copy();

    QRect rt = sa.rt();
    if (rt.width() == 0 || rt.height() == 0) {
        QPainter p(this);
        p.drawPixmap(0, 0, show_pix);
        return QWidget::paintEvent(e);
    }

    QPainter painter(&show_pix);
    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(2);
    pen.setStyle(Qt::DotLine);
    QFont font;
    font.setPixelSize(28);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(pen);
    painter.drawPixmap(rt.topLeft(), background.copy(rt));
    painter.drawRect(rt);
/*
    TODO
        1. 截图有边
        2. 添加箭头
        3. 箭头、矩形、文字以及调整边框互斥
*/
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.drawText(rt.x() + 2,
                     rt.y() - 8,
                     QStringLiteral("坐标：(%1, %2)  图片大小：(%3 x %4)")
                         .arg(rt.x())
                         .arg(rt.y())
                         .arg(rt.width())
                         .arg(rt.height()));

    // 绘制draw
    for (auto item : draw.items) {
        item.drawToPainter(&painter);
    }

    auto item = draw.cur_item;
    item.drawToPainter(&painter);
    // 文本类型，绘制一下虚线
    if (item.type == DrawType::Text) {
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        painter.drawRect(item.rt);
    }

    QPainter p(this);
    p.drawPixmap(0, 0, show_pix);
    return QWidget::paintEvent(e);
}

void ScreenShots::leaveEvent(QEvent *event)
{
    qInfo() << __func__;
    return QWidget::leaveEvent(event);
}

void ScreenShots::showButtons(QRect select_rect)
{
    // 在不同大小的多屏上还有点小问题，就先这样吧
    const int spacing = 20;
    const int buttons_x = qMax(0, select_rect.right() - buttons->width());
    int screen_h = QApplication::primaryScreen()->availableSize().height();
    bool show_in_area = (select_rect.bottom() + spacing + buttons->height() > screen_h);
    const int buttons_y = show_in_area ? select_rect.bottom() - spacing - buttons->height()
                                       : select_rect.bottom() + spacing;

    buttons->move(buttons_x, buttons_y);
    buttons->setVisible(true);
}

void ScreenShots::updateMouseShape(QPoint pos)
{
    if (draw.enable()) {
        setCursor(Qt::ArrowCursor);
    }
    else {
        auto border = sa.judgeBorder(pos);
        if (border != SelectShotArea::Border_None) {
            bool hor = border == SelectShotArea::Left || border == SelectShotArea::Right;
            setCursor(hor ? Qt::SizeHorCursor : Qt::SizeVerCursor);
        }
        else if (sa.isInArea(pos)) {
            setCursor(Qt::SizeAllCursor);
        }
    }
}
