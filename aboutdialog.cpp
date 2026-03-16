#include "aboutdialog.h"

#include "ui_aboutdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 复用主窗口样式，保持视觉一致。
    if (parent != nullptr)
    {
        setStyleSheet(parent->styleSheet());
    }

    connect(ui->copyInfoButton, &QPushButton::clicked, this, &AboutDialog::onCopyInfoClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);

    const QString buildTime = QStringLiteral(__DATE__) + QStringLiteral(" ") + QStringLiteral(__TIME__);
    ui->contentLabel->setText(QStringLiteral("应用：轻影截图（LightShadowCapture）\n"
                                             "版本：v0.3\n"
                                             "构建时间：%1\n"
                                             "功能：全屏截图、区域截图、历史记录、长截图")
                                 .arg(buildTime));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onCopyInfoClicked()
{
    const QString info = ui->contentLabel->text()
                         + QStringLiteral("\n复制时间：")
                         + QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QApplication::clipboard()->setText(info);
    ui->copyInfoButton->setText(QStringLiteral("已复制信息"));
}