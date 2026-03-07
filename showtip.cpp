#include "showtip.h"

#include "closedialog.h"
#include "ui_showtip.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

ShowTip::ShowTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowTip)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFocusPolicy(Qt::NoFocus);

    // 兜底样式：即便主窗口样式不继承到该顶层窗口，也保证提示清晰可见。
    ui->tipLabel->setStyleSheet(QStringLiteral("QLabel#tipLabel {"
                                               "min-height: 40px;"
                                               "min-width: 320px;"
                                               "color: #FFFFFF;"
                                               "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
                                               "stop:0 rgba(16, 36, 64, 246),"
                                               "stop:1 rgba(22, 52, 92, 246));"
                                               "border: 1px solid rgba(118, 182, 255, 228);"
                                               "border-radius: 14px;"
                                               "padding: 10px 18px;"
                                               "font-size: 12pt;"
                                               "font-weight: 700;"
                                               "}"));

    // 提升可见性：给提示内容增加柔和阴影。
    auto *shadow = new QGraphicsDropShadowEffect(ui->tipLabel);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(7, 20, 40, 150));
    ui->tipLabel->setGraphicsEffect(shadow);
}

ShowTip::~ShowTip()
{
    delete ui;
}

QSize ShowTip::measureSize(const QString &text)
{
    ui->tipLabel->setText(text);
    adjustSize();
    return size();
}

void ShowTip::showAt(const QString &text, const QPoint &globalTopLeft, int timeoutMs)
{
    ui->tipLabel->setText(text);
    adjustSize();

    move(globalTopLeft);
    show();
    raise();

    QTimer::singleShot(timeoutMs, this, &QWidget::hide);
}

ShowTip::CloseChoice ShowTip::askCloseChoice(QWidget *parent, const QString &appName)
{
    CloseDialog dialog(appName, parent);
    dialog.exec();

    switch (dialog.choice())
    {
        case CloseDialog::Choice::HideToTray:
            return CloseChoice::HideToTray;

        case CloseDialog::Choice::ExitApp:
            return CloseChoice::ExitApp;

        case CloseDialog::Choice::Cancel:
        default:
            return CloseChoice::Cancel;
    }
}
