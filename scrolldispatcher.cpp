#include "scrolldispatcher.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
void appendUniqueWindow(QVector<void *> *candidates, void *windowHandle)
{
    if (candidates == nullptr || windowHandle == nullptr || candidates->contains(windowHandle))
    {
        return;
    }

    candidates->append(windowHandle);
}
}

ScrollDispatcher::ScrollDispatcher()
{
}

bool ScrollDispatcher::resolveTarget(const QPoint &screenPos)
{
#ifdef Q_OS_WIN
    reset();

    HWND deepestWindow = static_cast<HWND>(resolveDeepestWindow(screenPos));
    if (deepestWindow == nullptr)
    {
        return false;
    }

    appendUniqueWindow(&m_targetCandidates, deepestWindow);

    HWND parentWindow = deepestWindow;
    while (parentWindow != nullptr)
    {
        parentWindow = GetParent(parentWindow);
        appendUniqueWindow(&m_targetCandidates, parentWindow);
    }

    const HWND rootWindow = GetAncestor(deepestWindow, GA_ROOT);
    appendUniqueWindow(&m_targetCandidates, rootWindow);

    if (m_targetCandidates.isEmpty())
    {
        return false;
    }

    m_targetIndex = 0;
    return true;
#else
    Q_UNUSED(screenPos)
    return false;
#endif
}

void ScrollDispatcher::reset()
{
    m_targetCandidates.clear();
    m_targetIndex = -1;
}

ScrollDispatchResult ScrollDispatcher::dispatchWheel(int delta, const QPoint &screenPos)
{
    ScrollDispatchResult result;
    result.injectionPoint = screenPos;

#ifdef Q_OS_WIN
    if (!hasTarget())
    {
        return result;
    }

    result.dispatched = dispatchWheelToWindow(m_targetCandidates.at(m_targetIndex), delta, screenPos);
#else
    Q_UNUSED(delta)
#endif

    return result;
}

bool ScrollDispatcher::hasTarget() const
{
#ifdef Q_OS_WIN
    return m_targetIndex >= 0
           && m_targetIndex < m_targetCandidates.size()
           && isValidWindow(m_targetCandidates.at(m_targetIndex));
#else
    return false;
#endif
}

bool ScrollDispatcher::advanceFallbackTarget()
{
#ifdef Q_OS_WIN
    for (int index = m_targetIndex + 1; index < m_targetCandidates.size(); ++index)
    {
        if (isValidWindow(m_targetCandidates.at(index)))
        {
            m_targetIndex = index;
            return true;
        }
    }
#endif

    return false;
}

#ifdef Q_OS_WIN
void *ScrollDispatcher::resolveDeepestWindow(const QPoint &screenPos) const
{
    POINT screenPoint;
    screenPoint.x = screenPos.x();
    screenPoint.y = screenPos.y();

    HWND window = WindowFromPoint(screenPoint);
    if (window == nullptr)
    {
        return nullptr;
    }

    HWND current = window;
    while (current != nullptr)
    {
        POINT clientPoint = screenPoint;
        ScreenToClient(current, &clientPoint);

        HWND child = ChildWindowFromPointEx(current,
                                            clientPoint,
                                            CWP_SKIPDISABLED | CWP_SKIPINVISIBLE);
        if (child == nullptr || child == current)
        {
            break;
        }

        current = child;
    }

    return current;
}

bool ScrollDispatcher::isValidWindow(void *windowHandle) const
{
    return windowHandle != nullptr
           && IsWindow(static_cast<HWND>(windowHandle))
           && IsWindowVisible(static_cast<HWND>(windowHandle));
}

bool ScrollDispatcher::dispatchWheelToWindow(void *windowHandle, int delta, const QPoint &screenPos) const
{
    if (!isValidWindow(windowHandle))
    {
        return false;
    }

    const LPARAM screenPoint = MAKELPARAM(static_cast<short>(screenPos.x()),
                                          static_cast<short>(screenPos.y()));
    const WPARAM wheelParam = MAKEWPARAM(0, static_cast<short>(delta));

    DWORD_PTR ignoredResult = 0;
    const LRESULT sendOk = SendMessageTimeoutW(static_cast<HWND>(windowHandle),
                                               WM_MOUSEWHEEL,
                                               wheelParam,
                                               screenPoint,
                                               SMTO_ABORTIFHUNG | SMTO_BLOCK,
                                               40,
                                               &ignoredResult);
    if (sendOk != 0)
    {
        return true;
    }

    const BOOL ok = PostMessageW(static_cast<HWND>(windowHandle),
                                 WM_MOUSEWHEEL,
                                 wheelParam,
                                 screenPoint);
    return ok != FALSE;
}
#endif
