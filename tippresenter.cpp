#include "tippresenter.h"

#include "showtip.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>

TipPresenter::TipPresenter() = default;

TipPresenter::~TipPresenter() = default;

void TipPresenter::show(const QString &text, TipPlacementPolicy policy, int timeoutMs)
{
    ensureView();

    if (!m_tipView)
    {
        return;
    }

    const QRect screenRect = resolveActiveScreenGeometry();
    const QSize tipSize = m_tipView->measureSize(text);

    QPoint targetPos = resolveBottomRightPosition(screenRect, tipSize);

    if (policy == TipPlacementPolicy::MousePreferredBottomRight)
    {
        constexpr int kMouseOffsetX = 18;
        constexpr int kMouseOffsetY = 20;

        const QPoint cursorPos = QCursor::pos();
        const QPoint candidate(cursorPos.x() + kMouseOffsetX,
                               cursorPos.y() + kMouseOffsetY);
        const QRect candidateRect(candidate, tipSize);

        if (screenRect.contains(candidateRect))
        {
            targetPos = candidate;
        }
    }

    m_tipView->showAt(text, targetPos, timeoutMs);
}

void TipPresenter::ensureView()
{
    if (!m_tipView)
    {
        // 作为独立顶层窗口存在，避免主窗口隐藏时提示不可见。
        m_tipView = std::make_unique<ShowTip>(nullptr);
    }
}

QRect TipPresenter::resolveActiveScreenGeometry() const
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }

    if (!screen)
    {
        const QList<QScreen *> screens = QGuiApplication::screens();
        if (!screens.isEmpty())
        {
            screen = screens.first();
        }
    }

    if (!screen)
    {
        return QRect(0, 0, 1920, 1080);
    }

    return screen->availableGeometry();
}

QPoint TipPresenter::resolveBottomRightPosition(const QRect &screenRect, const QSize &tipSize) const
{
    constexpr int kScreenMargin = 16;

    const int x = screenRect.x() + screenRect.width() - tipSize.width() - kScreenMargin;
    const int y = screenRect.y() + screenRect.height() - tipSize.height() - kScreenMargin;
    return QPoint(x, y);
}
