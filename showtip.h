/***********************************************************************************
*
* @file         showtip.h
* @brief        浮动提示组件：用于显示短时消息提示。
*
* @author       shanzhi
* @date         2026/03/06
* @history
***********************************************************************************/

#ifndef SHOWTIP_H
#define SHOWTIP_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ShowTip;
}
QT_END_NAMESPACE

/**
 * @brief 轻量提示窗口。
 *
 * 显示短时提示文本，超时后自动隐藏。
 */
class ShowTip : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造提示组件。
     * @param parent 父对象。
     */
    explicit ShowTip(QWidget *parent = nullptr);

    /**
     * @brief 析构提示组件。
     */
    ~ShowTip();

    /**
     * @brief 显示提示文本。
     * @param text 提示内容。
     * @param anchor 锚点窗口（用于定位提示位置）。
     * @param timeoutMs 自动隐藏超时时间（毫秒）。
     */
    void showText(const QString &text, QWidget *anchor, int timeoutMs = 1800);

private:
    Ui::ShowTip *ui; ///< UI 对象
};

#endif // SHOWTIP_H
