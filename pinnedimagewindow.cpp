#include "pinnedimagewindow.h"

#include <QCursor>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QToolButton>
#include <QWheelEvent>

namespace
{
constexpr qreal kMinScaleFactor = 0.18;
constexpr qreal kMaxScaleFactor = 3.0;
constexpr qreal kInitialScreenUsage = 0.42;
constexpr int kWindowMargin = 0;
constexpr int kSurfacePadding = 0;
constexpr int kCloseButtonSize = 18;
constexpr int kCloseButtonInset = 3;
constexpr int kSpawnOffset = 24;
}

PinnedImageWindow::PinnedImageWindow(const QPixmap &pixmap, QWidget *parent)
    : QWidget(parent)
    , m_surface(new QFrame(this))
    , m_imageLabel(new QLabel(m_surface))
    , m_closeButton(new QToolButton(this))
    , m_originalPixmap(pixmap)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

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

    m_closeButton->setObjectName(QStringLiteral("pinnedImageCloseButton"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setFixedSize(kCloseButtonSize, kCloseButtonSize);
    m_closeButton->setText(QStringLiteral("×"));
    m_closeButton->setToolTip(QStringLiteral("关闭贴图"));
    m_closeButton->setStyleSheet(
        "QToolButton#pinnedImageCloseButton {"
        "color: #F7FAFF;"
        "background: rgba(17, 24, 39, 180);"
        "border: 1px solid rgba(255, 255, 255, 64);"
        "border-radius: 10px;"
        "font: 10pt 'Microsoft YaHei UI';"
        "font-weight: 700;"
        "padding: 0px;"
        "}"
        "QToolButton#pinnedImageCloseButton:hover {"
        "background: rgba(226, 78, 97, 214);"
        "border-color: rgba(255, 255, 255, 108);"
        "}"
        "QToolButton#pinnedImageCloseButton:pressed {"
        "background: rgba(182, 48, 67, 224);"
        "}");
    connect(m_closeButton, &QToolButton::clicked, this, &QWidget::close);

    m_scaleFactor = initialScaleFactor();
    updateDisplayedPixmap();
    updateCloseButtonPosition();
}

void PinnedImageWindow::setPixmap(const QPixmap &pixmap)
{
    m_originalPixmap = pixmap;
    m_scaleFactor = initialScaleFactor();
    updateDisplayedPixmap();
    updateCloseButtonPosition();
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
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
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
    updateCloseButtonPosition();
}

void PinnedImageWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_initialPlacementPending)
    {
        return;
    }

    repositionWithinCurrentScreen(QCursor::pos() + QPoint(kSpawnOffset, kSpawnOffset));
    m_initialPlacementPending = false;
}

void PinnedImageWindow::wheelEvent(QWheelEvent *event)
{
    if (event == nullptr || m_originalPixmap.isNull())
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

void PinnedImageWindow::updateCloseButtonPosition()
{
    if (m_closeButton == nullptr || m_surface == nullptr)
    {
        return;
    }

    const QRect surfaceRect = m_imageLabel->geometry().translated(m_surface->pos());
    const int buttonX = surfaceRect.right() - kCloseButtonInset - m_closeButton->width() + 1;
    const int buttonY = surfaceRect.top() + kCloseButtonInset;
    m_closeButton->move(buttonX, buttonY);
    m_closeButton->raise();
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
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
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