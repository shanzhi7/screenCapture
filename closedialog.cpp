#include "closedialog.h"

#include "ui_closedialog.h"

CloseDialog::CloseDialog(const QString &appName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CloseDialog)
{
    ui->setupUi(this);

    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(size());

    // 复用主窗口样式表，保证弹窗与主界面视觉一致。
    if (parent != nullptr)
    {
        setStyleSheet(parent->styleSheet());
    }

    // 根据应用名动态更新提示文案，保持弹窗通用性。
    ui->messageLabel->setText(QStringLiteral("%1 仍在后台运行，选择一个关闭行为：").arg(appName));

    connect(ui->hideToTrayButton, &QPushButton::clicked, this, &CloseDialog::onHideToTrayClicked);
    connect(ui->exitAppButton, &QPushButton::clicked, this, &CloseDialog::onExitAppClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &CloseDialog::onCancelClicked);
}

CloseDialog::~CloseDialog()
{
    delete ui;
}

CloseDialog::Choice CloseDialog::choice() const
{
    return m_choice;
}

void CloseDialog::onHideToTrayClicked()
{
    m_choice = Choice::HideToTray;
    accept();
}

void CloseDialog::onExitAppClicked()
{
    m_choice = Choice::ExitApp;
    accept();
}

void CloseDialog::onCancelClicked()
{
    m_choice = Choice::Cancel;
    reject();
}
