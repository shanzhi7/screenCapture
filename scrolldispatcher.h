/***********************************************************************************
*
* @file         scrolldispatcher.h
* @brief        长截图滚轮投递器：负责目标窗口解析与滚轮消息投递。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef SCROLLDISPATCHER_H
#define SCROLLDISPATCHER_H

#include "longcapturetypes.h"

#include <QPoint>
#include <QVector>

class ScrollDispatcher
{
public:
    ScrollDispatcher();

    // 根据屏幕坐标解析滚动目标窗口。
    bool resolveTarget(const QPoint &screenPos);

    // 重置目标窗口状态。
    void reset();

    // 投递一档滚轮消息。
    ScrollDispatchResult dispatchWheel(int delta, const QPoint &screenPos);

    // 查询当前是否已锁定有效目标。
    bool hasTarget() const;

    // 切换到下一个候选目标窗口。
    bool advanceFallbackTarget();

private:
#ifdef Q_OS_WIN
    void *resolveDeepestWindow(const QPoint &screenPos) const;
    bool isValidWindow(void *windowHandle) const;
    bool dispatchWheelToWindow(void *windowHandle, int delta, const QPoint &screenPos) const;
#endif

private:
    QVector<void *> m_targetCandidates;
    int m_targetIndex = -1;
};

#endif // SCROLLDISPATCHER_H
