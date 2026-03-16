#include "pinnedimagewindow.h"

#include <QApplication>
#include <QCursor>
#include <cmath>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QSizePolicy>
#include <QToolButton>
#include <QWheelEvent>

namespace
{
constexpr qreal kMinScaleFactor = 0.18;
constexpr qreal kMaxScaleFactor = 3.0;
constexpr qreal kInitialScreenUsage = 0.42;
constexpr int kWindowMargin = 0;
constexpr int kSurfacePadding = 0;
constexpr int kControlButtonSize = 20;
constexpr int kControlButtonInset = 8;
constexpr int kControlButtonSpacing = 4;
constexpr int kSpawnOffset = 24;
constexpr int kMaximizedImagePadding = 12;

enum class WindowControlIconType
{
    Minimize,
    Maximize,
    Restore,
    Close
};

QIcon createWindowControlIcon(WindowControlIconType type, const QColor &color, const QSize &iconSize)
{
    const qreal devicePixelRatio = (qApp == nullptr) ? 1.0 : qApp->devicePixelRatio();
    const QSize pixelSize(qMax(1, qRound(iconSize.width() * devicePixelRatio)),
                          qMax(1, qRound(iconSize.height() * devicePixelRatio)));

    QPixmap pixmap(pixelSize);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color);
    pen.setWidthF(1.7);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    const QRectF bounds(QPointF(0.0, 0.0), QSizeF(iconSize.width(), iconSize.height()));
    const qreal left = bounds.left() + 2.2;
    const qreal right = bounds.right() - 2.2;
    const qreal top = bounds.top() + 2.2;
    const qreal bottom = bounds.bottom() - 2.2;
    const qreal width = right - left;
    const qreal height = bottom - top;

    switch (type)
    {
        case WindowControlIconType::Minimize:
            painter.drawLine(QPointF(left, bottom - 0.8), QPointF(right, bottom - 0.8));
            break;
        case WindowControlIconType::Maximize:
            painter.drawRect(QRectF(left + 0.5, top + 0.5, width - 1.0, height - 1.3));
            break;
        case WindowControlIconType::Restore:
            painter.drawRect(QRectF(left + width * 0.26,
                                    top + 0.4,
                                    width - width * 0.26 - 0.7,
                                    height - height * 0.26 - 0.9));
            painter.drawRect(QRectF(left,
                                    top + height * 0.28,
                                    width - width * 0.26 - 0.7,
                                    height - height * 0.28 - 0.7));
            break;
        case WindowControlIconType::Close:
            painter.drawLine(QPointF(left + 0.6, top + 0.6), QPointF(right - 0.6, bottom - 0.6));
            painter.drawLine(QPointF(right - 0.6, top + 0.6), QPointF(left + 0.6, bottom - 0.6));
            break;
    }

    return QIcon(pixmap);
}

QSize scaledPixmapSizeForBounds(const QSize &sourceSize, const QSize &bounds)
{
    if (!sourceSize.isValid() || !bounds.isValid())
    {
        return QSize();
    }

    return sourceSize.scaled(bounds, Qt::KeepAspectRatio);
}
}

PinnedImageWindow::PinnedImageWindow(const QPixmap &pixmap,
                                     const QString &title,
                                     const QPoint &spawnGlobalPos,
                                     QWidget *parent)
    : QWidget(parent)
    , m_surface(new QFrame(this))
    , m_imageLabel(new QLabel(m_surface))
    , m_minimizeButton(new QToolButton(this))
    , m_toggleMaximizeButton(new QToolButton(this))
    , m_closeButton(new QToolButton(this))
    , m_originalPixmap(pixmap)
    , m_spawnGlobalPos(spawnGlobalPos)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowTitle(title.trimmed().isEmpty() ? QStringLiteral("截图贴图") : title.trimmed());
    setWindowIcon(qApp == nullptr ? QIcon() : qApp->windowIcon());

    auto *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(kWindowMargin, kWindowMargin, kWindowMargin, kWindowMargin);
    outerLayout->setSpacing(0);
    outerLayout->addWidget(m_surface);

    m_surface->setObjectName(QStringLiteral("pinnedImageSurface"));
    m_surface->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_surface->setStyleSheet(
        "QFrame#pinnedImageSurface {"
        "background: transparent;"
        "border: none;"
        "}");

    auto *surfaceLayout = new QHBoxLayout(m_surface);
    surfaceLayout->setContentsMargins(kSurfacePadding, kSurfacePadding, kSurfacePadding, kSurfacePadding);
    surfaceLayout->setSpacing(0);
    surfaceLayout->addWidget(m_imageLabel, 0, Qt::AlignCenter);

    m_imageLabel->setObjectName(QStringLiteral("pinnedImageLabel"));
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_imageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_imageLabel->setStyleSheet(
        "QLabel#pinnedImageLabel {"
        "background: transparent;"
        "border: 1px solid rgba(214, 222, 234, 210);"
        "padding: 0px;"
        "}");

    const QString neutralControlStyle = QStringLiteral(
        "QToolButton {"
        "background: rgba(5, 10, 19, 226);"
        "border: 1px solid rgba(248, 251, 255, 120);"
        "border-radius: 10px;"
        "padding: 0px;"
        "}"
        "QToolButton:hover {"
        "background: rgba(38, 60, 96, 238);"
        "border-color: rgba(255, 255, 255, 170);"
        "}"
        "QToolButton:pressed {"
        "background: rgba(20, 33, 57, 244);"
        "}");

    const QString closeControlStyle = QStringLiteral(
        "QToolButton {"
        "background: rgba(104, 24, 39, 230);"
        "border: 1px solid rgba(255, 219, 225, 136);"
        "border-radius: 10px;"
        "padding: 0px;"
        "}"
        "QToolButton:hover {"
        "background: rgba(171, 42, 64, 242);"
        "border-color: rgba(255, 233, 237, 186);"
        "}"
        "QToolButton:pressed {"
        "background: rgba(129, 31, 49, 246);"
        "}");

    const QList<QToolButton *> controls = {m_minimizeButton, m_toggleMaximizeButton, m_closeButton};
    for (QToolButton *button : controls)
    {
        button->setCursor(Qt::PointingHandCursor);
        button->setFixedSize(kControlButtonSize, kControlButtonSize);
        button->setAutoRaise(false);
        button->setIconSize(QSize(12, 12));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    m_minimizeButton->setObjectName(QStringLiteral("pinnedImageMinimizeButton"));
    m_minimizeButton->setToolTip(QStringLiteral("最小化"));
    m_minimizeButton->setStyleSheet(neutralControlStyle);
    m_minimizeButton->setIcon(createWindowControlIcon(WindowControlIconType::Minimize,
                                                      QColor(QStringLiteral("#F8FBFF")),
                                                      m_minimizeButton->iconSize()));
    connect(m_minimizeButton, &QToolButton::clicked, this, &QWidget::showMinimized);

    m_toggleMaximizeButton->setObjectName(QStringLiteral("pinnedImageMaximizeButton"));
    m_toggleMaximizeButton->setToolTip(QStringLiteral("最大化"));
    m_toggleMaximizeButton->setStyleSheet(neutralControlStyle);
    connect(m_toggleMaximizeButton, &QToolButton::clicked, this, [this]()
    {
        toggleMaximizeRestore();
    });

    m_closeButton->setObjectName(QStringLiteral("pinnedImageCloseButton"));
    m_closeButton->setToolTip(QStringLiteral("关闭贴图"));
    m_closeButton->setStyleSheet(closeControlStyle);
    m_closeButton->setIcon(createWindowControlIcon(WindowControlIconType::Close,
                                                   QColor(QStringLiteral("#FFF7F8")),
                                                   m_closeButton->iconSize()));
    connect(m_closeButton, &QToolButton::clicked, this, &QWidget::close);

    m_scaleFactor = initialScaleFactor();
    updateDisplayedPixmap();
    updateMaximizeButtonState();
    updateWindowControlsPosition();
}

void PinnedImageWindow::setPixmap(const QPixmap &pixmap)
{
    m_originalPixmap = pixmap;
    m_scaleFactor = initialScaleFactor();
    m_restoreScaleFactor = m_scaleFactor;
    m_isCustomMaximized = false;
    updateDisplayedPixmap();
    updateMaximizeButtonState();
    updateWindowControlsPosition();
}

void PinnedImageWindow::mousePressEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        close();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        m_dragging = true;
        m_dragStartGlobalPos = event->globalPosition().toPoint();
        m_dragOffset = m_isCustomMaximized
                           ? event->position().toPoint()
                           : (m_dragStartGlobalPos - frameGeometry().topLeft());
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void PinnedImageWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event == nullptr || !m_dragging)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPoint globalPos = event->globalPosition().toPoint();
    if (m_isCustomMaximized)
    {
        if ((globalPos - m_dragStartGlobalPos).manhattanLength() < QApplication::startDragDistance())
        {
            event->accept();
            return;
        }

        const qreal anchorRatio = (width() <= 0)
                                      ? 0.5
                                      : qBound(0.0,
                                               static_cast<qreal>(m_dragOffset.x()) / qMax(1, width()),
                                               1.0);
        const int dragOffsetY = qBound(0, m_dragOffset.y(), qMax(0, height() - 1));

        m_isCustomMaximized = false;
        m_scaleFactor = m_restoreScaleFactor;
        updateDisplayedPixmap();
        updateMaximizeButtonState();
        updateWindowControlsPosition();

        const int restoredAnchorX = qRound(width() * anchorRatio);
        const int restoredAnchorY = qBound(0, dragOffsetY, qMax(0, height() - 1));
        repositionWithinCurrentScreen(QPoint(globalPos.x() - restoredAnchorX,
                                             globalPos.y() - restoredAnchorY));
        m_dragOffset = globalPos - frameGeometry().topLeft();
    }

    move(globalPos - m_dragOffset);
    event->accept();
}

void PinnedImageWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event != nullptr && event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void PinnedImageWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event != nullptr && event->button() == Qt::LeftButton)
    {
        close();
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void PinnedImageWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateDisplayedPixmap();
    updateWindowControlsPosition();
}

void PinnedImageWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_initialPlacementPending)
    {
        return;
    }

    const QPoint preferredPos = m_spawnGlobalPos.isNull()
                                    ? (QCursor::pos() + QPoint(kSpawnOffset, kSpawnOffset))
                                    : (m_spawnGlobalPos + QPoint(kSpawnOffset, kSpawnOffset));
    repositionWithinCurrentScreen(preferredPos);
    m_initialPlacementPending = false;
}

void PinnedImageWindow::wheelEvent(QWheelEvent *event)
{
    if (event == nullptr || m_originalPixmap.isNull() || m_isCustomMaximized)
    {
        QWidget::wheelEvent(event);
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0)
    {
        QWidget::wheelEvent(event);
        return;
    }

    const QRectF beforeImageRect = displayedImageRect();
    if (!beforeImageRect.isValid() || beforeImageRect.width() <= 0.0 || beforeImageRect.height() <= 0.0)
    {
        QWidget::wheelEvent(event);
        return;
    }

    const QPointF localAnchor = event->position();
    const QPointF clampedAnchor(qBound(beforeImageRect.left(), localAnchor.x(), beforeImageRect.right()),
                                qBound(beforeImageRect.top(), localAnchor.y(), beforeImageRect.bottom()));
    const QPoint anchorFloor(static_cast<int>(std::floor(clampedAnchor.x())),
                             static_cast<int>(std::floor(clampedAnchor.y())));
    const QPointF globalAnchor = QPointF(mapToGlobal(anchorFloor))
                                 + QPointF(clampedAnchor.x() - anchorFloor.x(),
                                           clampedAnchor.y() - anchorFloor.y());

    const qreal beforeScaleX = beforeImageRect.width() / qMax(1, m_originalPixmap.width());
    const qreal beforeScaleY = beforeImageRect.height() / qMax(1, m_originalPixmap.height());
    const QPointF imageAnchor((clampedAnchor.x() - beforeImageRect.left()) / beforeScaleX,
                              (clampedAnchor.y() - beforeImageRect.top()) / beforeScaleY);

    const qreal previousScale = m_scaleFactor;
    if (delta > 0)
    {
        m_scaleFactor = qMin(kMaxScaleFactor, m_scaleFactor * 1.12);
    }
    else
    {
        m_scaleFactor = qMax(kMinScaleFactor, m_scaleFactor / 1.12);
    }

    if (qFuzzyCompare(previousScale, m_scaleFactor))
    {
        event->accept();
        return;
    }

    updateDisplayedPixmap();
    if (layout() != nullptr)
    {
        layout()->activate();
    }
    if (m_surface != nullptr && m_surface->layout() != nullptr)
    {
        m_surface->layout()->activate();
    }

    const QRectF afterImageRect = displayedImageRect();
    if (!afterImageRect.isValid() || afterImageRect.width() <= 0.0 || afterImageRect.height() <= 0.0)
    {
        event->accept();
        return;
    }

    const qreal afterScaleX = afterImageRect.width() / qMax(1, m_originalPixmap.width());
    const qreal afterScaleY = afterImageRect.height() / qMax(1, m_originalPixmap.height());
    const QPointF anchorInWindow(afterImageRect.left() + imageAnchor.x() * afterScaleX,
                                 afterImageRect.top() + imageAnchor.y() * afterScaleY);
    const QPointF newTopLeft = globalAnchor - anchorInWindow;
    move(qRound(newTopLeft.x()), qRound(newTopLeft.y()));
    event->accept();
}

QRectF PinnedImageWindow::displayedImageRect() const
{
    if (m_surface == nullptr || m_imageLabel == nullptr || !m_displayedPixmapSize.isValid())
    {
        return QRectF();
    }

    QRectF contentsRect(m_imageLabel->contentsRect());
    contentsRect.translate(m_imageLabel->pos());
    contentsRect.translate(m_surface->pos());
    if (!contentsRect.isValid() || contentsRect.width() <= 0.0 || contentsRect.height() <= 0.0)
    {
        return QRectF();
    }

    const QSize visibleSize = m_displayedPixmapSize.scaled(contentsRect.size().toSize(),
                                                           Qt::KeepAspectRatio);
    if (!visibleSize.isValid())
    {
        return QRectF();
    }

    const QPointF topLeft(contentsRect.left() + (contentsRect.width() - visibleSize.width()) * 0.5,
                          contentsRect.top() + (contentsRect.height() - visibleSize.height()) * 0.5);
    return QRectF(topLeft, QSizeF(visibleSize));
}

void PinnedImageWindow::updateDisplayedPixmap()
{
    if (m_originalPixmap.isNull())
    {
        m_displayedPixmapSize = QSize();
        m_imageLabel->setPixmap(QPixmap());
        return;
    }

    QSize targetSize;
    if (m_isCustomMaximized)
    {
        const QRect available = availableGeometry();
        const QSize bounds = available.size() - QSize(kMaximizedImagePadding * 2,
                                                      kMaximizedImagePadding * 2);
        targetSize = scaledPixmapSizeForBounds(m_originalPixmap.size(), bounds);
        if (m_originalPixmap.width() > 0 && m_originalPixmap.height() > 0 && targetSize.isValid())
        {
            const qreal widthScale = static_cast<qreal>(targetSize.width()) / m_originalPixmap.width();
            const qreal heightScale = static_cast<qreal>(targetSize.height()) / m_originalPixmap.height();
            m_scaleFactor = qMax(kMinScaleFactor, qMin(kMaxScaleFactor, qMin(widthScale, heightScale)));
        }
    }
    else
    {
        targetSize = m_originalPixmap.size() * m_scaleFactor;
    }

    const QPixmap scaled = m_originalPixmap.scaled(targetSize,
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation);
    m_displayedPixmapSize = scaled.size();
    m_imageLabel->setPixmap(scaled);
    m_imageLabel->setFixedSize(scaled.size());

    if (!m_isCustomMaximized)
    {
        adjustSize();
    }
}

void PinnedImageWindow::updateWindowControlsPosition()
{
    if (m_minimizeButton == nullptr || m_toggleMaximizeButton == nullptr || m_closeButton == nullptr)
    {
        return;
    }

    const int controlY = kControlButtonInset;
    const int closeX = width() - kControlButtonInset - m_closeButton->width();
    const int toggleX = closeX - kControlButtonSpacing - m_toggleMaximizeButton->width();
    const int minimizeX = toggleX - kControlButtonSpacing - m_minimizeButton->width();

    m_closeButton->move(closeX, controlY);
    m_toggleMaximizeButton->move(toggleX, controlY);
    m_minimizeButton->move(minimizeX, controlY);

    m_minimizeButton->raise();
    m_toggleMaximizeButton->raise();
    m_closeButton->raise();
}

void PinnedImageWindow::updateMaximizeButtonState()
{
    if (m_toggleMaximizeButton == nullptr)
    {
        return;
    }

    if (m_isCustomMaximized)
    {
        m_toggleMaximizeButton->setToolTip(QStringLiteral("还原"));
        m_toggleMaximizeButton->setIcon(createWindowControlIcon(WindowControlIconType::Restore,
                                                                QColor(QStringLiteral("#F8FBFF")),
                                                                m_toggleMaximizeButton->iconSize()));
        return;
    }

    m_toggleMaximizeButton->setToolTip(QStringLiteral("最大化"));
    m_toggleMaximizeButton->setIcon(createWindowControlIcon(WindowControlIconType::Maximize,
                                                            QColor(QStringLiteral("#F8FBFF")),
                                                            m_toggleMaximizeButton->iconSize()));
}

void PinnedImageWindow::toggleMaximizeRestore()
{
    if (m_isCustomMaximized)
    {
        restoreFromMaximized();
        return;
    }

    maximizeToCurrentScreen();
}

void PinnedImageWindow::maximizeToCurrentScreen()
{
    if (m_originalPixmap.isNull())
    {
        return;
    }

    m_restoreGeometry = frameGeometry();
    m_restoreScaleFactor = m_scaleFactor;
    m_isCustomMaximized = true;

    const QRect available = availableGeometry();
    setGeometry(available);
    updateDisplayedPixmap();
    updateMaximizeButtonState();
    updateWindowControlsPosition();
}

void PinnedImageWindow::restoreFromMaximized()
{
    if (!m_restoreGeometry.isValid())
    {
        return;
    }

    m_isCustomMaximized = false;
    m_scaleFactor = m_restoreScaleFactor;
    updateDisplayedPixmap();
    move(m_restoreGeometry.topLeft());
    updateMaximizeButtonState();
    updateWindowControlsPosition();
}

void PinnedImageWindow::repositionWithinCurrentScreen(const QPoint &preferredTopLeft)
{
    const QRect available = availableGeometry();
    int x = preferredTopLeft.x();
    int y = preferredTopLeft.y();

    x = qBound(available.left(), x, qMax(available.left(), available.right() - width()));
    y = qBound(available.top(), y, qMax(available.top(), available.bottom() - height()));
    move(x, y);
}

QRect PinnedImageWindow::availableGeometry() const
{
    QScreen *screen = QGuiApplication::screenAt(frameGeometry().center());
    if (screen == nullptr)
    {
        screen = QGuiApplication::screenAt(m_spawnGlobalPos);
    }
    if (screen == nullptr)
    {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }
    if (screen == nullptr)
    {
        screen = QGuiApplication::primaryScreen();
    }

    return (screen == nullptr) ? QRect(0, 0, 1280, 720) : screen->availableGeometry();
}

qreal PinnedImageWindow::initialScaleFactor() const
{
    if (m_originalPixmap.isNull())
    {
        return 1.0;
    }

    const QRect available = availableGeometry();
    const qreal maxWidth = qMax(220.0, available.width() * kInitialScreenUsage);
    const qreal maxHeight = qMax(180.0, available.height() * kInitialScreenUsage);
    const qreal widthScale = maxWidth / m_originalPixmap.width();
    const qreal heightScale = maxHeight / m_originalPixmap.height();
    return qBound(kMinScaleFactor, qMin<qreal>(1.0, qMin(widthScale, heightScale)), kMaxScaleFactor);
}

qreal PinnedImageWindow::maximumScaleFactorForAvailableGeometry(const QRect &available) const
{
    if (m_originalPixmap.isNull() || available.width() <= 0 || available.height() <= 0)
    {
        return m_scaleFactor;
    }

    const qreal widthScale = static_cast<qreal>(available.width()) / m_originalPixmap.width();
    const qreal heightScale = static_cast<qreal>(available.height()) / m_originalPixmap.height();
    return qMax(kMinScaleFactor, qMin(widthScale, heightScale));
}
