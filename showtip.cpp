#include "showtip.h"

#include "ui_showtip.h"

#include <QLabel>
#include <QPoint>
#include <QTimer>

ShowTip::ShowTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowTip)
{
    ui->setupUi(this);

    // 作为轻量浮层，不参与任务栏，不抢占主窗口焦点。
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
}

ShowTip::~ShowTip()
{
    delete ui;
}

void ShowTip::showText(const QString &text, QWidget *anchor, int timeoutMs)
{
    if (anchor == nullptr)
    {
        return;
    }

    ui->tipLabel->setText(text);
    adjustSize();

    // 默认显示在锚点窗口右上区域，尽量不遮挡核心内容区。
    const QPoint global = anchor->mapToGlobal(QPoint(anchor->width() - width() - 28, 24));
    move(global);
    show();
    raise();

    // 超时自动隐藏，避免打断连续操作。
    QTimer::singleShot(timeoutMs, this, &QWidget::hide);
}
