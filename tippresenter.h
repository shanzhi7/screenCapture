/***********************************************************************************
*
* @file         tippresenter.h
* @brief        提示协调器：统一处理提示定位策略与展示生命周期。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef TIPPRESENTER_H
#define TIPPRESENTER_H

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>

#include <memory>

class ShowTip;

// 提示位置策略。
enum class TipPlacementPolicy
{
    MousePreferredBottomRight,
    ScreenBottomRight
};

class TipPresenter
{
public:
    TipPresenter();
    ~TipPresenter();

    // 显示提示：默认“鼠标优先”，越界回退到屏幕右下角。
    void show(const QString &text,
              TipPlacementPolicy policy = TipPlacementPolicy::MousePreferredBottomRight,
              int timeoutMs = 1800);

private:
    // 懒加载提示视图，避免主窗口生命周期耦合。
    void ensureView();

    // 获取鼠标所在屏幕可用区域，失败时回退主屏。
    QRect resolveActiveScreenGeometry() const;

    // 计算屏幕右下角提示坐标。
    QPoint resolveBottomRightPosition(const QRect &screenRect, const QSize &tipSize) const;

private:
    std::unique_ptr<ShowTip> m_tipView;
};

#endif // TIPPRESENTER_H
