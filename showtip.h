/***********************************************************************************
*
* @file         showtip.h
* @brief        浮动提示视图：仅负责提示内容展示，不承担定位策略。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef SHOWTIP_H
#define SHOWTIP_H

#include <QPoint>
#include <QSize>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ShowTip;
}
QT_END_NAMESPACE

class ShowTip : public QWidget
{
    Q_OBJECT

public:
    enum class CloseChoice
    {
        HideToTray,
        ExitApp,
        Cancel
    };

public:
    explicit ShowTip(QWidget *parent = nullptr);
    ~ShowTip();

    // 计算指定文本在当前样式下的提示框尺寸。
    QSize measureSize(const QString &text);

    // 在指定全局坐标显示短时提示文本（坐标为窗口左上角）。
    void showAt(const QString &text, const QPoint &globalTopLeft, int timeoutMs = 1800);

    // 截图期间抑制提示窗口展示（并隐藏当前所有提示窗口）。
    static void setCaptureSuppressed(bool suppressed);

    // 查询当前是否处于截图提示抑制状态。
    static bool isCaptureSuppressed();

    // 统一关闭行为弹窗：隐藏到托盘 / 退出应用 / 取消。
    static CloseChoice askCloseChoice(QWidget *parent, const QString &appName);

private:
    Ui::ShowTip *ui;
};

#endif // SHOWTIP_H
