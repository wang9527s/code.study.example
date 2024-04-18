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
    initButtons();
    setStyleSheet("QPushButton{font: 25px;}"
                  "QPushButton { background-color: #CCCCCC; border: 1px solid #999999; } "
                  "QPushButton:hover { background-color: #DDDDDD; }"
                  "QPushButton:checked { background-color: #AAAAAA; } "
                  "QPushButton:pressed { background-color: #888888; }"

                  "QLineEdit{background:transparent;border: 2px solid #0078D7;}");

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_StaticContents); // 设置内容静态，避免不必要的重绘
    // Qt::WindowTransparentForInput
    // 窗口不接收输入事件，所以用户不能与其进行交互。
    // Qt::X11BypassWindowManagerHint
    setWindowFlags(Qt::SplashScreen // 不显示任务栏图标，Qt::Tool 会导致失去键盘焦点
                   | Qt::FramelessWindowHint    // 无标题栏
                   | Qt::WindowStaysOnTopHint); // 置顶
    setMouseTracking(true);
}

void ScreenShots::initFrame()
{
    background = Tool::mergeGrabWindow();

    bool debug = false;
    if (debug) {
        background.fill(Qt::transparent);
        grayBackground = background.copy();
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
    resize(background.size());
    setVisible(true);
    setFocus();
}

void ScreenShots::on_save_image()
{
    QString path = QCoreApplication::applicationDirPath();
    QString name = QDateTime::currentDateTime().toString("yyyyMMdd-hh-mm-ss");
    save_and_exit(path, name);
}

void ScreenShots::inputMethodEvent(QInputMethodEvent *event)
{
    // 获取输入法的输出
    QString txt = event->commitString();

    if (!txt.isNull()) {
        inputText += txt;
        update();
    }
    QWidget::inputMethodEvent(event);
}

void ScreenShots::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        qApp->quit();
    }
    else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S) {
        QString desktopPath =
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

        Tool::copyPixmapToClipboard(show_pix.copy(sa.rt()));
        QString path = QFileDialog::getExistingDirectory(this, u8"选择保存路径", desktopPath);
        QString name = QDateTime::currentDateTime().toString("yyyyMMdd-hh-mm-ss");
        save_and_exit(path, name);
    }

    // 和inputMethodEvent配合，完成输入文字以及字符的功能
    if (operate_type == OperateType::DrawText) {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            inputText += '\n';
        }
        else if (event->key() == Qt::Key_Backspace) {
            inputText.remove(inputText.length() - 1, 1);
            update();
        }
        else {
            QString in = event->text();
            if (in != "") {
                inputText += in;
                update();
            }
        }
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

        if (operate_type == OperateType::SelectArea) {
            sa.mousePressed(pos);
        }
        else {
            press_pos = pos;
            if (inputText != "") {
                items.append(new DrawItemText(inputText, rt));
                inputText.clear();
            }

            // 给TextItem类型 一个初始值  TODO优化
            rt = QRect(pos, QSize(200, 150));
        }
    }
    if (e->type() == QEvent::MouseButtonRelease) {
        isPress = false;
        if (operate_type == OperateType::SelectArea) {
            sa.mouseRelease();
        }
        else {
            if (operate_type == OperateType::DrawRect) {
                items.append(new DrawItemRect(rt));
            }
            else if (operate_type == OperateType::DrawLineArrow) {
                items.append(new DrawItemLineArraw(press_pos, end_pos));
            }
        }

        setCursor(Qt::ArrowCursor);
    }
    if (e->type() == QEvent::MouseMove) {
        if (isPress) {
            if (operate_type == OperateType::SelectArea) {
                // 移动截图区域和操作按钮
                sa.mouseMove(pos);
                showButtons(sa.rt());
            }
            else {
                rt = Tool::rt(press_pos, pos);
                end_pos = pos;
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

    QRect sa_rt = sa.rt();
    if (sa_rt.width() == 0 || sa_rt.height() == 0) {
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
    painter.drawPixmap(sa_rt.topLeft(), background.copy(sa_rt));

    pen.setColor(Qt::black);
    painter.setPen(pen);
    QString msg = QString(u8"图片大小：(%1 x %2)").arg(sa_rt.width()).arg(sa_rt.height());
    painter.drawText(sa_rt.x() + 2, sa_rt.y() - 8, msg);

    // 绘制draw
    for (auto item : items) {
        item->drawToPainter(&painter);
    }

    // 绘制当前item
    DrawItemBase *pItem = nullptr;
    if (OperateType::DrawRect == operate_type) {
        pItem = new DrawItemRect(rt);
    }
    else if (OperateType::DrawText == operate_type) {
        pItem = new DrawItemText(inputText, rt);

        // 绘制一下虚线
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        painter.drawRect(rt);
    }
    else if (OperateType::DrawLineArrow == operate_type) {
        pItem = new DrawItemLineArraw(press_pos, end_pos);
    }

    if (pItem)
        pItem->drawToPainter(&painter);
    delete pItem;

    QPainter p(this);
    p.drawPixmap(0, 0, show_pix);
    p.drawRect(sa_rt);
    return QWidget::paintEvent(e);
}

void ScreenShots::initButtons()
{
    QPushButton *pOk = new QPushButton;
    QPushButton *pCancel = new QPushButton;

    QPushButton *select_rect = new QPushButton;
    QPushButton *draw_text = new QPushButton;
    QPushButton *draw_rect = new QPushButton;
    QPushButton *draw_array = new QPushButton;
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(select_rect, OperateType::SelectArea);
    buttonGroup->addButton(draw_text, OperateType::DrawText);
    buttonGroup->addButton(draw_rect, OperateType::DrawRect);
    buttonGroup->addButton(draw_array, OperateType::DrawLineArrow);

    QPushButton *draw_back = new QPushButton;

    pOk->setIcon(QIcon(":/icon/ok.png"));
    pCancel->setIcon(QIcon(":/icon/cancel.png"));
    draw_back->setIcon(QIcon(":/icon/back.png"));
    select_rect->setIcon(QIcon(":/icon/select-rect.png"));
    draw_text->setIcon(QIcon(":/icon/text.png"));
    draw_rect->setIcon(QIcon(":/icon/draw-rect.png"));
    draw_array->setIcon(QIcon(":/icon/line-arrow.png"));

    pOk->setToolTip(u8"保存到本地和剪切板");
    select_rect->setToolTip(u8"调整截屏区域");
    draw_back->setToolTip(u8"撤销");
    draw_rect->setToolTip(u8"绘制矩形");
    draw_text->setToolTip(u8"添加文本");
    draw_array->setToolTip(u8"绘制矩形");

    buttons = new QWidget(this);
    buttons->setVisible(false);
    pOk->setFixedSize(80, 40);
    pCancel->setFixedSize(80, 40);
    buttons->setFixedSize(500, 40);
    QHBoxLayout *buttons_pl = new QHBoxLayout(buttons);
    buttons_pl->setMargin(0);
    buttons_pl->addStretch();
    buttons_pl->addWidget(draw_back);
    buttons_pl->addSpacing(20);
    buttons_pl->addWidget(select_rect);
    buttons_pl->addWidget(draw_rect);
    buttons_pl->addWidget(draw_text);
    buttons_pl->addWidget(draw_array);
    buttons_pl->addSpacing(20);
    buttons_pl->addWidget(pCancel);
    buttons_pl->addWidget(pOk);

    for (QWidget *p : buttons->findChildren<QWidget *>()) {
        p->setFixedSize(40, 50);
    }
    select_rect->setChecked(true);
    for (QAbstractButton *p : buttonGroup->buttons()) {
        p->setCheckable(true);
    }

    connect(pOk, &QPushButton::clicked, this, &ScreenShots::on_save_image);
    connect(pCancel, &QPushButton::clicked, this, [=] { qApp->quit(); });
    connect(draw_back, &QPushButton::clicked, this, [=] {
        if (operate_type == OperateType::DrawText && inputText != "") {
            inputText = "";
        }
        else {
            if (items.size() >= 1)
                items.pop_back();
        }
        update();
    });

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [&](int id) {
        if (OperateType::DrawText == operate_type) {
            if (inputText != "") {
                items.append(new DrawItemText(inputText, rt));
                inputText.clear();
            }
        }
        operate_type = OperateType(id);
    });
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
    if (operate_type == OperateType::SelectArea) {
        auto border = sa.judgeBorder(pos);
        if (border != SelectShotArea::Border_None) {
            bool hor = border == SelectShotArea::Left || border == SelectShotArea::Right;
            setCursor(hor ? Qt::SizeHorCursor : Qt::SizeVerCursor);
        }
        else if (sa.isInArea(pos)) {
            setCursor(Qt::SizeAllCursor);
        }
    }
    else {
        setCursor(Qt::ArrowCursor);
    }
}

void ScreenShots::save_and_exit(QString path, QString name)
{
    QString pathname = QString("%1/%2.png").arg(path).arg(name);
    QPixmap pix = show_pix.copy(sa.rt());
    pix.save(pathname);
    QDesktopServices::openUrl(pathname);

    qApp->quit();
}
