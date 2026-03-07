#include "capturesettingsdialog.h"

#include "ui_capturesettingsdialog.h"

#include <QDir>
#include <QFileDialog>

CaptureSettingsDialog::CaptureSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CaptureSettingsDialog)
{
    ui->setupUi(this);

    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 复用主窗口样式，保持风格一致。
    if (parent != nullptr)
    {
        setStyleSheet(parent->styleSheet());
    }

    connect(ui->browseDirButton, &QPushButton::clicked, this, &CaptureSettingsDialog::onBrowseDirectoryClicked);
    connect(ui->clearDirButton, &QPushButton::clicked, this, &CaptureSettingsDialog::onClearDirectoryClicked);
    connect(ui->resetHotkeyButton, &QPushButton::clicked, this, &CaptureSettingsDialog::onResetHotkeyClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

CaptureSettingsDialog::~CaptureSettingsDialog()
{
    delete ui;
}

void CaptureSettingsDialog::setCurrentHotkey(const QKeySequence &sequence)
{
    if (sequence.isEmpty())
    {
        ui->hotkeyEdit->setKeySequence(QKeySequence(QStringLiteral("Ctrl+Shift+A")));
        return;
    }

    ui->hotkeyEdit->setKeySequence(sequence);
}

void CaptureSettingsDialog::setAutoSaveDirectory(const QString &directory)
{
    const QString trimmed = directory.trimmed();
    ui->autoSaveDirEdit->setText(QDir::toNativeSeparators(trimmed));
}

QKeySequence CaptureSettingsDialog::selectedHotkey() const
{
    return ui->hotkeyEdit->keySequence();
}

QString CaptureSettingsDialog::selectedAutoSaveDirectory() const
{
    return QDir::fromNativeSeparators(ui->autoSaveDirEdit->text().trimmed());
}

void CaptureSettingsDialog::onBrowseDirectoryClicked()
{
    const QString current = selectedAutoSaveDirectory();
    const QString chosen = QFileDialog::getExistingDirectory(this,
                                                             QStringLiteral("选择自动保存目录"),
                                                             current,
                                                             QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (chosen.isEmpty())
    {
        return;
    }

    ui->autoSaveDirEdit->setText(QDir::toNativeSeparators(QDir(chosen).absolutePath()));
}

void CaptureSettingsDialog::onClearDirectoryClicked()
{
    ui->autoSaveDirEdit->clear();
}

void CaptureSettingsDialog::onResetHotkeyClicked()
{
    ui->hotkeyEdit->setKeySequence(QKeySequence(QStringLiteral("Ctrl+Shift+A")));
}
