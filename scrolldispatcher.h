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

    bool resolveTarget(const QPoint &screenPos);
    void reset();
    ScrollDispatchResult dispatchWheel(int delta, const QPoint &screenPos);
    bool hasTarget() const;
    bool advanceFallbackTarget();

private:
#ifdef Q_OS_WIN
    void *resolveDeepestWindow(const QPoint &screenPos) const;
    bool isValidWindow(void *windowHandle) const;
    bool dispatchWheelToWindow(void *windowHandle, int delta, const QPoint &screenPos) const;
    bool dispatchWheelByInput(void *windowHandle, int delta, const QPoint &screenPos) const;
    void activateTargetWindow(void *windowHandle) const;
#endif

private:
    QVector<void *> m_targetCandidates;
    int m_targetIndex = -1;
};

#endif // SCROLLDISPATCHER_H
