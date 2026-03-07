/***********************************************************************************
*
* @file         closedialog.h
* @brief        关闭行为确认对话框：提供隐藏到托盘、退出应用、取消三种选择。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef CLOSEDIALOG_H
#define CLOSEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class CloseDialog;
}
QT_END_NAMESPACE

class CloseDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Choice
    {
        HideToTray,
        ExitApp,
        Cancel
    };

public:
    explicit CloseDialog(const QString &appName, QWidget *parent = nullptr);
    ~CloseDialog();

    // 返回用户最终选择，供外层窗口统一处理关闭行为。
    Choice choice() const;

private slots:
    // 选择“隐藏到系统托盘”并关闭对话框。
    void onHideToTrayClicked();

    // 选择“退出应用”并关闭对话框。
    void onExitAppClicked();

    // 选择“取消”并关闭对话框。
    void onCancelClicked();

private:
    Ui::CloseDialog *ui;
    Choice m_choice = Choice::Cancel;
};

#endif // CLOSEDIALOG_H
