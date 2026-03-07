/***********************************************************************************
*
* @file         capturesettingsdialog.h
* @brief        截图设置对话框：用于编辑截图热键与自动保存目录。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef CAPTURESETTINGSDIALOG_H
#define CAPTURESETTINGSDIALOG_H

#include <QDialog>
#include <QKeySequence>

QT_BEGIN_NAMESPACE
namespace Ui
{
class CaptureSettingsDialog;
}
QT_END_NAMESPACE

class CaptureSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureSettingsDialog(QWidget *parent = nullptr);
    ~CaptureSettingsDialog() override;

    // 设置当前截图快捷键。
    void setCurrentHotkey(const QKeySequence &sequence);

    // 设置自动保存目录（空字符串表示使用默认目录）。
    void setAutoSaveDirectory(const QString &directory);

    // 获取用户在对话框中选择的快捷键。
    QKeySequence selectedHotkey() const;

    // 获取用户在对话框中选择的自动保存目录。
    QString selectedAutoSaveDirectory() const;

private slots:
    // 选择自动保存目录。
    void onBrowseDirectoryClicked();

    // 清空目录配置，回退默认目录策略。
    void onClearDirectoryClicked();

    // 恢复默认快捷键。
    void onResetHotkeyClicked();

private:
    Ui::CaptureSettingsDialog *ui;
};

#endif // CAPTURESETTINGSDIALOG_H
