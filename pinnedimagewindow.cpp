#include "pinnedimagewindow.h"

#include <QApplication>
#include <QCursor>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QStyle>
#include <QToolButton>
#include <QWheelEvent>

namespace
{
constexpr qreal kMinScaleFactor = 0.18;
constexpr qreal kMaxScaleFactor = 3.0;
constexpr qreal kInitialScreenUsage = 0.42;
constexpr int kWindowMargin = 0;
constexpr int kSurfacePadding = 0;
constexpr int kControlButtonSize = 18;
constexpr int kControlButtonInset = 3;
constexpr int kControlButtonSpacing = 4;
constexpr int kSpawnOffset = 24;
constexpr int kMaximizedPadding = 18;
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
    surfaceLayout->addWidget(m_imageLabel);

    m_imageLabel->setObjectName(QStringLiteral("pinnedImageLabel"));
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_imageLabel->setStyleSheet(
        "QLabel#pinnedImageLabel {"
        "background: transparent;"
        "border: 1px solid rgba(214, 222, 234, 210);"
        "padding: 0px;"
        "}");

    const QString controlStyle = QStringLiteral(
        "QToolButton {"
        "color: #F7FAFF;"
        "background: rgba(17, 24, 39, 180);"
        "border: 1px solid rgba(255, 255, 255, 64);"
        "border-radius: 9px;"
        "padding: 0px;"
        "}"
        "QToolButton:hover {"
        "background: rgba(52, 76, 118, 214);"
        "border-color: rgba(255, 255, 255, 108);"
        "}"
        "QToolButton:pressed {"
        "background: rgba(33, 52, 86, 224);"
        "}");

    const QList<QToolButton *> controls = {m_minimizeButton, m_toggleMaximizeButton, m_closeButton};
    for (QToolButton *button : controls)
    {
        button->setCursor(Qt::PointingHandCursor);
        button->setFixedSize(kControlButtonSize, kControlButtonSize);
        button->setStyleSheet(controlStyle);
        button->setAutoRaise(false);
        button->setIconSize(QSize(12, 12));
    }

    m_minimizeButton->setObjectName(QStringLiteral("pinnedImageMinimizeButton"));
    m_minimizeButton->setToolTip(QStringLiteral("最小化"));
    m_minimizeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    connect(m_minimizeButton, &QToolButton::clicked, this, &QWidget::showMinimized);

    m_toggleMaximizeButton->setObjectName(QStringLiteral("pinnedImageMaximizeButton"));
    connect(m_toggleMaximizeButton, &QToolButton::clicked, this, [this]()
    {
        toggleMaximizeRestore();
    });

    m_closeButton->setObjectName(QStringLiteral("pinnedImageCloseButton"));
    m_closeButton->setToolTip(QStringLiteral("关闭贴图"));
    m_closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
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

    if (event->button() == Qt::LeftButton && !m_isCustomMaximized)
    {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void PinnedImageWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event == nullptr || !m_dragging || m_isCustomMaximized)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    move(event->globalPosition().toPoint() - m_dragOffset);
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

    const QPoint center = frameGeometry().center();
    if (delta > 0)
    {
        m_scaleFactor = qMin(kMaxScaleFactor, m_scaleFactor * 1.12);
    }
    else
    {
        m_scaleFactor = qMax(kMinScaleFactor, m_scaleFactor / 1.12);
    }

    updateDisplayedPixmap();
    repositionWithinCurrentScreen(center - rect().center());
    event->accept();
}

void PinnedImageWindow::updateDisplayedPixmap()
{
    if (m_originalPixmap.isNull())
    {
        m_imageLabel->setPixmap(QPixmap());
        return;
    }

    const QSize scaledSize = m_originalPixmap.size() * m_scaleFactor;
    const QPixmap scaled = m_originalPixmap.scaled(scaledSize,
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation);
    m_imageLabel->setPixmap(scaled);
    m_imageLabel->setFixedSize(scaled.size());
    adjustSize();
}

void PinnedImageWindow::updateWindowControlsPosition()
{
    if (m_minimizeButton == nullptr || m_toggleMaximizeButton == nullptr || m_closeButton == nullptr)
    {
        return;
    }

    const QRect imageRect = m_imageLabel->geometry().translated(m_surface->pos());
    const int closeX = imageRect.right() - kControlButtonInset - m_closeButton->width() + 1;
    const int controlY = imageRect.top() + kControlButtonInset;
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
        m_toggleMaximizeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
        return;
    }

    m_toggleMaximizeButton->setToolTip(QStringLiteral("最大化"));
    m_toggleMaximizeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
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

    const QRect available = availableGeometry().adjusted(kMaximizedPadding,
                                                         kMaximizedPadding,
                                                         -kMaximizedPadding,
                                                         -kMaximizedPadding);
    m_scaleFactor = maximumScaleFactorForAvailableGeometry(available);
    updateDisplayedPixmap();

    const QPoint centeredTopLeft = available.center() - rect().center();
    repositionWithinCurrentScreen(centeredTopLeft);
    m_isCustomMaximized = true;
    updateMaximizeButtonState();
    updateWindowControlsPosition();
}

void PinnedImageWindow::restoreFromMaximized()
{
    if (!m_restoreGeometry.isValid())
    {
        return;
    }

    m_scaleFactor = m_restoreScaleFactor;
    updateDisplayedPixmap();
    setGeometry(m_restoreGeometry);
    m_isCustomMaximized = false;
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