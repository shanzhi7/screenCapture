#include "selectionoverlay.h"

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
#include "longcapturepreviewpanel.h"
#endif

#include <QAbstractButton>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QContextMenuEvent>
#include <QCursor>
#include <QEvent>
#include <QFont>
#include <QFontMetricsF>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QScreen>
#include <QShortcut>
#include <QToolButton>
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
#include <QWheelEvent>
#endif

namespace
{
constexpr QSize kCursorInfoPanelSize(176, 154);
constexpr int kCursorInfoPanelOffset = 18;
constexpr int kCursorInfoPanelMargin = 10;
constexpr int kCursorMagnifierGrid = 11;
constexpr int kCursorInfoBottomHeight = 56;
constexpr qreal kOverlayPenWidth = 3.2;
constexpr int kOverlayMosaicBlockSize = 12;
constexpr qreal kOverlayTextPixelSize = 20.0;
constexpr qreal kOverlayPenWidths[3] = {2.6, 4.8, 8.0};
constexpr qreal kOverlayShapeWidths[3] = {2.0, 4.0, 6.0};
constexpr qreal kOverlayMosaicBrushSizes[3] = {18.0, 28.0, 40.0};
constexpr int kOverlayMosaicBlockSizes[3] = {8, 12, 18};
constexpr qreal kOverlayTextSizes[3] = {18.0, 24.0, 32.0};
constexpr int kOverlayTextEditorMinWidth = 120;
constexpr int kOverlayTextEditorMaxWidth = 280;
constexpr int kOverlayTextEditorHeight = 34;
constexpr int kOverlayTextEditorMargin = 6;
const QColor kDefaultAnnotationColor(255, 96, 110);

QFont annotationTextFont(qreal pixelSize)
{
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPixelSize(qMax(12, qRound(pixelSize)));
    font.setWeight(QFont::DemiBold);
    return font;
}

QCursor penToolCursor()
{
    QIcon icon(QStringLiteral(":/icons/overlay_pen.svg"));
    if (icon.isNull())
    {
        return QCursor(Qt::CrossCursor);
    }

    const QPixmap pixmap = icon.pixmap(24, 24);
    if (pixmap.isNull())
    {
        return QCursor(Qt::CrossCursor);
    }

    return QCursor(pixmap, 3, 21);
}

QRect cursorInfoPanelRect(const QRect &bounds, const QPoint &cursorPos)
{
    int panelX = cursorPos.x() + kCursorInfoPanelOffset;
    int panelY = cursorPos.y() + kCursorInfoPanelOffset;

    if (panelX + kCursorInfoPanelSize.width() > bounds.right() - kCursorInfoPanelMargin)
    {
        panelX = cursorPos.x() - kCursorInfoPanelSize.width() - kCursorInfoPanelOffset;
    }

    if (panelY + kCursorInfoPanelSize.height() > bounds.bottom() - kCursorInfoPanelMargin)
    {
        panelY = cursorPos.y() - kCursorInfoPanelSize.height() - kCursorInfoPanelOffset;
    }

    const int minX = bounds.left() + kCursorInfoPanelMargin;
    const int maxX = qMax(minX, bounds.right() - kCursorInfoPanelSize.width() - kCursorInfoPanelMargin);
    const int minY = bounds.top() + kCursorInfoPanelMargin;
    const int maxY = qMax(minY, bounds.bottom() - kCursorInfoPanelSize.height() - kCursorInfoPanelMargin);

    panelX = qBound(minX, panelX, maxX);
    panelY = qBound(minY, panelY, maxY);
    return QRect(QPoint(panelX, panelY), kCursorInfoPanelSize);
}

QPoint clampedSamplePoint(const QImage &image, const QPoint &point)
{
    if (image.isNull())
    {
        return QPoint();
    }

    return QPoint(qBound(0, point.x(), qMax(0, image.width() - 1)),
                  qBound(0, point.y(), qMax(0, image.height() - 1)));
}

QImage magnifierSample(const QImage &image, const QPoint &center)
{
    QImage sample(kCursorMagnifierGrid, kCursorMagnifierGrid, QImage::Format_ARGB32_Premultiplied);
    sample.fill(QColor(18, 24, 35));

    if (image.isNull())
    {
        return sample;
    }

    const int radius = kCursorMagnifierGrid / 2;
    for (int y = 0; y < kCursorMagnifierGrid; ++y)
    {
        for (int x = 0; x < kCursorMagnifierGrid; ++x)
        {
            const QPoint sourcePoint = clampedSamplePoint(image, center + QPoint(x - radius, y - radius));
            sample.setPixelColor(x, y, image.pixelColor(sourcePoint));
        }
    }

    return sample;
}

QString hexColorText(const QColor &color)
{
    return color.name(QColor::HexRgb).toUpper();
}

void drawCursorInfoPanel(QPainter *painter,
                         const QRect &bounds,
                         const QPoint &cursorPos,
                         const QPoint &globalCursorPos,
                         const QImage &desktopSnapshot)
{
    if (painter == nullptr || desktopSnapshot.isNull() || !bounds.contains(cursorPos))
    {
        return;
    }

    const QRect panelRect = cursorInfoPanelRect(bounds, cursorPos);
    const QRect contentRect = panelRect.adjusted(8, 8, -8, -8);
    const QRect magnifierRect(contentRect.left(),
                              contentRect.top(),
                              contentRect.width(),
                              qMax(48, contentRect.height() - kCursorInfoBottomHeight - 6));
    const QRect infoRect(contentRect.left(),
                         magnifierRect.bottom() + 6,
                         contentRect.width(),
                         qMax(44, contentRect.bottom() - magnifierRect.bottom() - 6));

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(14, 20, 30, 236));
    painter->drawRoundedRect(panelRect, 12, 12);
    painter->setPen(QPen(QColor(136, 185, 255, 96), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(panelRect.adjusted(0, 0, -1, -1), 12, 12);

    const QImage zoomed = magnifierSample(desktopSnapshot, cursorPos).scaled(magnifierRect.size(),
                                                                              Qt::IgnoreAspectRatio,
                                                                              Qt::FastTransformation);
    painter->save();
    painter->setClipRect(magnifierRect.adjusted(1, 1, -1, -1));
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawImage(magnifierRect, zoomed);
    painter->restore();

    painter->setPen(QPen(QColor(228, 239, 255, 34), 1.0));
    const double cellWidth = magnifierRect.width() / static_cast<double>(kCursorMagnifierGrid);
    const double cellHeight = magnifierRect.height() / static_cast<double>(kCursorMagnifierGrid);
    for (int index = 1; index < kCursorMagnifierGrid; ++index)
    {
        const int gridX = qRound(magnifierRect.left() + cellWidth * index);
        const int gridY = qRound(magnifierRect.top() + cellHeight * index);
        painter->drawLine(gridX, magnifierRect.top(), gridX, magnifierRect.bottom());
        painter->drawLine(magnifierRect.left(), gridY, magnifierRect.right(), gridY);
    }

    const int centerIndex = kCursorMagnifierGrid / 2;
    QRectF centerCell(magnifierRect.left() + cellWidth * centerIndex,
                      magnifierRect.top() + cellHeight * centerIndex,
                      cellWidth,
                      cellHeight);
    painter->setPen(QPen(QColor(41, 255, 157, 230), 1.6));
    painter->drawRect(centerCell.adjusted(0.8, 0.8, -0.8, -0.8));
    painter->drawLine(QPointF(centerCell.center().x(), magnifierRect.top()),
                      QPointF(centerCell.center().x(), magnifierRect.bottom()));
    painter->drawLine(QPointF(magnifierRect.left(), centerCell.center().y()),
                      QPointF(magnifierRect.right(), centerCell.center().y()));

    painter->setPen(QPen(QColor(136, 185, 255, 56), 1.0));
    painter->drawLine(infoRect.left(), infoRect.top() - 3, infoRect.right(), infoRect.top() - 3);

    const QColor sampledColor = desktopSnapshot.pixelColor(clampedSamplePoint(desktopSnapshot, cursorPos));
    const QString colorText = hexColorText(sampledColor);
    const QString coordText = QStringLiteral("%1, %2").arg(globalCursorPos.x()).arg(globalCursorPos.y());

    QFont labelFont = painter->font();
    labelFont.setFamily(QStringLiteral("Microsoft YaHei UI"));
    labelFont.setPointSizeF(8.8);
    labelFont.setWeight(QFont::Normal);

    QFont valueFont = labelFont;
    valueFont.setPointSizeF(9.6);
    valueFont.setWeight(QFont::DemiBold);

    const QRect coordRow(infoRect.left(), infoRect.top(), infoRect.width(), 18);
    const QRect colorRow(infoRect.left(), infoRect.top() + 20, infoRect.width(), 18);
    const QRect hintRow(infoRect.left(), infoRect.bottom() - 18, infoRect.width(), 18);

    painter->setFont(labelFont);
    painter->setPen(QColor(206, 220, 244, 170));
    painter->drawText(coordRow, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("坐标"));
    painter->drawText(colorRow, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("色值"));

    painter->setFont(valueFont);
    painter->setPen(QColor(248, 251, 255));
    painter->drawText(coordRow, Qt::AlignRight | Qt::AlignVCenter, coordText);
    painter->drawText(colorRow, Qt::AlignRight | Qt::AlignVCenter, colorText);

    painter->setFont(labelFont);
    painter->setPen(QColor(206, 220, 244, 154));
    painter->drawText(hintRow, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("按 Ctrl + C 复制色值"));
    painter->restore();
}
}

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : QWidget(parent)
{
    QRect virtualDesktop;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
    {
        if (screen == nullptr)
        {
            continue;
        }

        virtualDesktop = virtualDesktop.isNull()
                             ? screen->geometry()
                             : virtualDesktop.united(screen->geometry());
    }

    if (virtualDesktop.isNull())
    {
        virtualDesktop = QRect(0, 0, 1280, 720);
    }

    setGeometry(virtualDesktop);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    m_cursorPos = mapFromGlobal(QCursor::pos());

    auto *cancelShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    cancelShortcut->setContext(Qt::ApplicationShortcut);
    connect(cancelShortcut, &QShortcut::activated, this, [this]()
    {
        emit selectionCanceled();
        close();
    });

    auto *confirmShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    confirmShortcut->setContext(Qt::ApplicationShortcut);
    connect(confirmShortcut, &QShortcut::activated, this, &SelectionOverlay::confirmSelection);

    auto *confirmShortcutEnter = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    confirmShortcutEnter->setContext(Qt::ApplicationShortcut);
    connect(confirmShortcutEnter, &QShortcut::activated, this, &SelectionOverlay::confirmSelection);

    ensureToolbar();
    resetAnnotationHistory();
    captureDesktopSnapshot();
    updateActiveCursor();
}

QRect SelectionOverlay::selectedRect() const
{
    return m_selectedRect;
}

QPixmap SelectionOverlay::applyEditsToPixmap(const QPixmap &pixmap, const QRect &captureRect) const
{
    Q_UNUSED(captureRect)

    if (pixmap.isNull() || (m_penStrokes.isEmpty() && m_rectangles.isEmpty() && m_ellipses.isEmpty() && m_mosaics.isEmpty() && m_texts.isEmpty()))
    {
        return pixmap;
    }

    const QRect referenceRect = editableSelectionRect();
    if (!referenceRect.isValid())
    {
        return pixmap;
    }

    QPixmap result = pixmap;
    QImage mosaicSource;
    if (!m_mosaics.isEmpty())
    {
        mosaicSource = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        if (mosaicSource.isNull())
        {
            return pixmap;
        }
    }

    QPainter painter(&result);
    if (!painter.isActive())
    {
        return pixmap;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    const QRect outputRect(QPoint(0, 0), result.size());
    if (!m_mosaics.isEmpty())
    {
        paintMosaics(&painter, outputRect, referenceRect, mosaicSource, false);
    }
    paintPenStrokes(&painter, outputRect, referenceRect, false);
    paintRectangles(&painter, outputRect, referenceRect, false);
    paintEllipses(&painter, outputRect, referenceRect, false);
    paintTexts(&painter, outputRect, referenceRect);
    painter.end();

    return result;
}

void SelectionOverlay::captureDesktopSnapshot()
{
    if (width() <= 0 || height() <= 0)
    {
        m_desktopSnapshot = QImage();
        return;
    }

    m_desktopSnapshot = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    m_desktopSnapshot.fill(Qt::transparent);

    QPainter painter(&m_desktopSnapshot);
    const QPoint overlayTopLeft = geometry().topLeft();
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
    {
        if (screen == nullptr)
        {
            continue;
        }

        const QPixmap screenshot = screen->grabWindow(0);
        if (screenshot.isNull())
        {
            continue;
        }

        QImage screenImage = screenshot.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        if (screenImage.size() != screen->geometry().size())
        {
            screenImage = screenImage.scaled(screen->geometry().size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
        }

        painter.drawImage(screen->geometry().topLeft() - overlayTopLeft, screenImage);
    }
}

QString SelectionOverlay::copyCurrentColorValue()
{
    if (m_desktopSnapshot.isNull() || !rect().contains(m_cursorPos) || (m_hasSelection && !m_selecting))
    {
        return QString();
    }

    const QColor sampledColor = m_desktopSnapshot.pixelColor(clampedSamplePoint(m_desktopSnapshot, m_cursorPos));
    const QString colorText = hexColorText(sampledColor);
    QApplication::clipboard()->setText(colorText);
    return colorText;
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void SelectionOverlay::setLongCaptureModeEnabled(bool enabled)
{
    const bool targetEnabled = enabled && m_hasSelection;
    m_longCaptureEnabled = targetEnabled;

    if (m_btnLongCapture != nullptr)
    {
        const bool blocked = m_btnLongCapture->blockSignals(true);
        m_btnLongCapture->setChecked(targetEnabled);
        m_btnLongCapture->blockSignals(blocked);
    }

    if (!targetEnabled)
    {
        m_predictedLongCaptureHeight = currentRect().height();
        m_committedLongCaptureHeight = currentRect().height();
        m_longCaptureAnchorBottomLocal = -1;
        m_captureQuality = CaptureQuality::Idle;
        if (m_previewPanel != nullptr)
        {
            m_previewPanel->setVisualHeights(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
            m_previewPanel->hide();
        }
    }
    else
    {
        const QRect current = currentRect();
        m_longCaptureAnchorBottomLocal = current.bottom();
        m_committedLongCaptureHeight = qMax(current.height(), m_committedLongCaptureHeight);
        m_predictedLongCaptureHeight = qMax(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);

        if (m_previewPanel != nullptr)
        {
            m_previewPanel->setVisualHeights(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
            updatePreviewPanelPosition();
            m_previewPanel->show();
            m_previewPanel->raise();
        }
    }

    updateStatusLabel();
    updateToolbarPosition();
    updateActiveCursor();
    update();
}

void SelectionOverlay::setPredictedLongCaptureHeight(int height)
{
    m_predictedLongCaptureHeight = qMax(m_committedLongCaptureHeight, height);
    if (m_previewPanel != nullptr)
    {
        m_previewPanel->setVisualHeights(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
    }

    if (m_hasSelection)
    {
        updatePreviewPanelPosition();
        update();
    }
}

void SelectionOverlay::setCommittedLongCaptureHeight(int height)
{
    m_committedLongCaptureHeight = qMax(0, height);
    if (m_predictedLongCaptureHeight < m_committedLongCaptureHeight)
    {
        m_predictedLongCaptureHeight = m_committedLongCaptureHeight;
    }

    if (m_previewPanel != nullptr)
    {
        m_previewPanel->setVisualHeights(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
    }

    if (m_hasSelection)
    {
        updateToolbarPosition();
        updatePreviewPanelPosition();
        update();
    }
}

void SelectionOverlay::setLongCapturePreview(const QPixmap &preview)
{
    m_previewPixmap = preview;
    if (m_previewPanel == nullptr)
    {
        return;
    }

    if (m_previewPixmap.isNull())
    {
        m_previewPanel->clearPreview();
        m_previewPanel->setVisualHeights(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
        if (!m_longCaptureEnabled)
        {
            m_previewPanel->hide();
        }
        return;
    }

    m_previewPanel->setPreview(m_previewPixmap);
    m_previewPanel->setVisualHeights(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
    if (m_longCaptureEnabled)
    {
        updatePreviewPanelPosition();
        m_previewPanel->show();
        m_previewPanel->raise();
    }
}

void SelectionOverlay::setStatusText(const QString &text)
{
    m_statusText = text.trimmed();
    updateStatusLabel();
}

void SelectionOverlay::setCaptureQuality(CaptureQuality quality)
{
    m_captureQuality = quality;
    updateStatusLabel();
    update();
}

void SelectionOverlay::setCaptureDecorationsHidden(bool hidden)
{
    if (m_captureDecorationsHidden == hidden)
    {
        return;
    }

    m_captureDecorationsHidden = hidden;
    const QRect captureRect = currentRect().adjusted(-2, -2, 2, 2);
    if (hidden)
    {
        m_toolbarVisibleBeforeCapture = (m_toolbar != nullptr
                                         && m_toolbar->isVisible()
                                         && m_toolbar->geometry().intersects(captureRect));
        if (m_toolbarVisibleBeforeCapture)
        {
            m_toolbar->hide();
        }

        m_styleToolbarVisibleBeforeCapture = (m_styleToolbar != nullptr
                                              && m_styleToolbar->isVisible()
                                              && m_styleToolbar->geometry().intersects(captureRect));
        if (m_styleToolbarVisibleBeforeCapture)
        {
            m_styleToolbar->hide();
        }

        m_statusLabelVisibleBeforeCapture = (m_statusLabel != nullptr
                                             && m_statusLabel->isVisible()
                                             && m_statusLabel->geometry().intersects(captureRect));
        if (m_statusLabelVisibleBeforeCapture)
        {
            m_statusLabel->hide();
        }

        m_previewPanelVisibleBeforeCapture = (m_previewPanel != nullptr
                                              && m_previewPanel->isVisible()
                                              && m_previewPanel->geometry().intersects(captureRect));
        if (m_previewPanelVisibleBeforeCapture)
        {
            m_previewPanel->hide();
        }
    }
    else
    {
        bool toolbarRestored = false;
        if (m_toolbarVisibleBeforeCapture && m_toolbar != nullptr && m_hasSelection)
        {
            updateToolbarPosition();
            toolbarRestored = true;
        }
        m_toolbarVisibleBeforeCapture = false;

        if (!toolbarRestored
            && m_styleToolbarVisibleBeforeCapture
            && m_styleToolbar != nullptr
            && m_hasSelection
            && toolSupportsStyleToolbar(m_activeTool))
        {
            updateStyleToolbarPosition();
        }
        m_styleToolbarVisibleBeforeCapture = false;

        if (m_statusLabelVisibleBeforeCapture && m_statusLabel != nullptr && m_hasSelection)
        {
            updateStatusPosition();
        }
        m_statusLabelVisibleBeforeCapture = false;

        if (m_previewPanelVisibleBeforeCapture && m_previewPanel != nullptr && m_longCaptureEnabled)
        {
            updatePreviewPanelPosition();
            m_previewPanel->show();
            m_previewPanel->raise();
        }
        m_previewPanelVisibleBeforeCapture = false;
    }

    update();
}
#endif

void SelectionOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    m_cursorPos = event->pos();

    if (event->button() == Qt::RightButton)
    {
        event->accept();
        emit selectionCanceled();
        close();
        return;
    }

    if (event->button() != Qt::LeftButton)
    {
        event->accept();
        return;
    }

    activateWindow();
    raise();
    setFocus();

    if ((m_toolbar != nullptr && m_toolbar->isVisible() && m_toolbar->geometry().contains(event->pos()))
        || (m_styleToolbar != nullptr && m_styleToolbar->isVisible() && m_styleToolbar->geometry().contains(event->pos())))
    {
        QWidget::mousePressEvent(event);
        return;
    }

    if (shouldHandlePenAt(event->pos()))
    {
        m_drawingPenStroke = true;
        m_currentPenColor = m_penColor;
        m_currentPenWidth = penWidthForIndex(m_penThicknessIndex);
        m_currentPenStroke.clear();
        m_currentPenStroke.append(clampPointToSelection(event->pos()));
        event->accept();
        update();
        return;
    }

    if (shouldHandleRectangleAt(event->pos()))
    {
        m_drawingRectangle = true;
        m_currentRectangleColor = m_rectangleColor;
        m_currentRectangleStrokeWidth = shapeStrokeWidthForIndex(m_rectangleThicknessIndex);
        m_currentRectangleAnchor = clampPointToSelection(event->pos());
        m_currentRectangle = QRectF(m_currentRectangleAnchor, m_currentRectangleAnchor);
        event->accept();
        update();
        return;
    }

    if (shouldHandleEllipseAt(event->pos()))
    {
        m_drawingEllipse = true;
        m_currentEllipseColor = m_ellipseColor;
        m_currentEllipseStrokeWidth = shapeStrokeWidthForIndex(m_ellipseThicknessIndex);
        m_currentEllipseAnchor = clampPointToSelection(event->pos());
        m_currentEllipse = QRectF(m_currentEllipseAnchor, m_currentEllipseAnchor);
        event->accept();
        update();
        return;
    }

    if (shouldHandleMosaicAt(event->pos()))
    {
        m_drawingMosaic = true;
        m_currentMosaicBrushSize = mosaicBrushSizeForIndex(m_mosaicThicknessIndex);
        m_currentMosaicBlockSize = mosaicBlockSizeForIndex(m_mosaicThicknessIndex);
        m_currentMosaicStroke.clear();
        m_currentMosaicStroke.append(clampPointToSelection(event->pos()));
        event->accept();
        update();
        return;
    }

    if (shouldHandleTextAt(event->pos()))
    {
        beginTextEditingAt(event->pos());
        event->accept();
        return;
    }

    if (shouldHandleToolAt(event->pos()))
    {
        event->accept();
        return;
    }

    clearAnnotations();
    m_selecting = true;
    m_hasSelection = false;
    m_startPos = event->pos();
    m_endPos = m_startPos;
    m_selectedRect = QRect();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_longCaptureEnabled = false;
    m_predictedLongCaptureHeight = 0;
    m_committedLongCaptureHeight = 0;
    m_longCaptureAnchorBottomLocal = -1;
    m_captureQuality = CaptureQuality::Idle;
    m_statusText = QStringLiteral("拖动中");
    m_previewPixmap = QPixmap();

    if (m_btnLongCapture != nullptr)
    {
        const bool blocked = m_btnLongCapture->blockSignals(true);
        m_btnLongCapture->setChecked(false);
        m_btnLongCapture->blockSignals(blocked);
    }

    if (m_previewPanel != nullptr)
    {
        m_previewPanel->clearPreview();
        m_previewPanel->hide();
    }

    updateStatusLabel();
#else
    if (m_statusLabel != nullptr)
    {
        m_statusLabel->setText(QStringLiteral("拖动中"));
    }
#endif

    if (m_toolbar != nullptr)
    {
        m_toolbar->hide();
    }
    if (m_styleToolbar != nullptr)
    {
        m_styleToolbar->hide();
    }
    if (m_statusLabel != nullptr)
    {
        m_statusLabel->hide();
    }

    updateActiveCursor();
    event->accept();
    update();
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    m_cursorPos = event->pos();

    if (m_drawingPenStroke)
    {
        const QPointF point = clampPointToSelection(event->pos());
        if (m_currentPenStroke.isEmpty() || m_currentPenStroke.constLast() != point)
        {
            m_currentPenStroke.append(point);
        }
        event->accept();
        update();
        return;
    }

    if (m_drawingRectangle)
    {
        const QPointF point = clampPointToSelection(event->pos());
        m_currentRectangle = QRectF(m_currentRectangleAnchor, point).normalized();
        event->accept();
        update();
        return;
    }

    if (m_drawingEllipse)
    {
        const QPointF point = clampPointToSelection(event->pos());
        m_currentEllipse = QRectF(m_currentEllipseAnchor, point).normalized();
        event->accept();
        update();
        return;
    }

    if (m_drawingMosaic)
    {
        const QPointF point = clampPointToSelection(event->pos());
        if (m_currentMosaicStroke.isEmpty() || m_currentMosaicStroke.constLast() != point)
        {
            m_currentMosaicStroke.append(point);
        }
        event->accept();
        update();
        return;
    }

    if (m_selecting)
    {
        m_endPos = event->pos();
        event->accept();
        update();
        return;
    }

    QWidget::mouseMoveEvent(event);
    update();
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    m_cursorPos = event->pos();

    if (event->button() == Qt::RightButton)
    {
        event->accept();
        return;
    }

    if (m_drawingPenStroke && event->button() == Qt::LeftButton)
    {
        const QPointF point = clampPointToSelection(event->pos());
        if (m_currentPenStroke.isEmpty() || m_currentPenStroke.constLast() != point)
        {
            m_currentPenStroke.append(point);
        }
        finishCurrentPenStroke();
        event->accept();
        update();
        return;
    }

    if (m_drawingRectangle && event->button() == Qt::LeftButton)
    {
        const QPointF point = clampPointToSelection(event->pos());
        m_currentRectangle = QRectF(m_currentRectangleAnchor, point).normalized();
        finishCurrentRectangle();
        event->accept();
        update();
        return;
    }

    if (m_drawingEllipse && event->button() == Qt::LeftButton)
    {
        const QPointF point = clampPointToSelection(event->pos());
        m_currentEllipse = QRectF(m_currentEllipseAnchor, point).normalized();
        finishCurrentEllipse();
        event->accept();
        update();
        return;
    }

    if (m_drawingMosaic && event->button() == Qt::LeftButton)
    {
        const QPointF point = clampPointToSelection(event->pos());
        if (m_currentMosaicStroke.isEmpty() || m_currentMosaicStroke.constLast() != point)
        {
            m_currentMosaicStroke.append(point);
        }
        finishCurrentMosaic();
        event->accept();
        update();
        return;
    }

    if (!m_selecting || event->button() != Qt::LeftButton)
    {
        event->accept();
        return;
    }

    m_endPos = event->pos();
    m_selecting = false;

    const QRect rect = currentRect();
    if (rect.width() < 2 || rect.height() < 2)
    {
        resetSelection();
        event->accept();
        return;
    }

    m_hasSelection = true;
    m_selectedRect = toGlobalRect(rect);
    updateHistoryButtons();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_committedLongCaptureHeight = rect.height();
    m_predictedLongCaptureHeight = rect.height();
    m_longCaptureAnchorBottomLocal = rect.bottom();
    m_captureQuality = CaptureQuality::Idle;
    m_statusText = QStringLiteral("已选择区域");
    updateStatusLabel();
#else
    if (m_statusLabel != nullptr)
    {
        m_statusLabel->setText(QStringLiteral("已选择区域"));
    }
#endif

    updateToolbarPosition();
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    updatePreviewPanelPosition();
#endif

    activateWindow();
    raise();
    setFocus();

    updateActiveCursor();
    event->accept();
    update();
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (event->matches(QKeySequence::Copy))
    {
        const QString colorText = copyCurrentColorValue();
        if (!colorText.isEmpty())
        {
            emit colorValueCopied(colorText);
            close();
            return;
        }
    }

    if (event->matches(QKeySequence::Undo))
    {
        undoLastAnnotation();
        return;
    }

    if (event->matches(QKeySequence::Redo))
    {
        redoLastAnnotation();
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        emit selectionCanceled();
        close();
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        confirmSelection();
        return;
    }

    QWidget::keyPressEvent(event);
}

void SelectionOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const bool hideCaptureDecorations = m_captureDecorationsHidden;
    const QRect committedSelection = currentDisplayRect();
    const QRect predictedSelection = predictedDisplayRect();
    const QRect visibleSelection = predictedSelection.isValid() ? predictedSelection : committedSelection;
#else
    const bool hideCaptureDecorations = false;
    const QRect committedSelection = currentRect();
    const QRect visibleSelection = committedSelection;
#endif
    const bool validSelection = (m_selecting || m_hasSelection) && visibleSelection.width() > 1 && visibleSelection.height() > 1;
    const bool showCursorInfoPanel = !hideCaptureDecorations && !m_selecting && !m_hasSelection;

    QPainterPath overlayPath;
    overlayPath.addRect(rect());

    if (validSelection)
    {
        QPainterPath holePath;
        holePath.addRect(visibleSelection);
        overlayPath = overlayPath.subtracted(holePath);
    }

    painter.fillPath(overlayPath, QColor(0, 0, 0, 122));

    if (!validSelection)
    {
        painter.setPen(QColor(220, 230, 255, 160));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("拖动鼠标选择区域，右键或 ESC 取消"));

        if (showCursorInfoPanel)
        {
            drawCursorInfoPanel(&painter, rect(), m_cursorPos, mapToGlobal(m_cursorPos), m_desktopSnapshot);
        }
        return;
    }

    painter.fillRect(visibleSelection, QColor(0, 0, 0, 1));

    if (hideCaptureDecorations)
    {
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    if (m_longCaptureEnabled && predictedSelection != committedSelection)
    {
        QPen predictedPen(QColor(155, 203, 255, 165), 1.0, Qt::DashLine);
        predictedPen.setDashPattern({6.0, 4.0});
        painter.setPen(predictedPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(predictedSelection.adjusted(0, 0, -1, -1), 4, 4);
    }
#endif

    painter.setPen(QPen(QColor(76, 166, 255, 215), 1.4));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(committedSelection.adjusted(0, 0, -1, -1), 4, 4);
    paintMosaics(&painter, committedSelection, editableSelectionRect(), m_desktopSnapshot, true);
    paintPenStrokes(&painter, committedSelection, editableSelectionRect(), true);
    paintRectangles(&painter, committedSelection, editableSelectionRect(), true);
    paintEllipses(&painter, committedSelection, editableSelectionRect(), true);
    paintTexts(&painter, committedSelection, editableSelectionRect());

    QString sizeText = QString::number(committedSelection.width())
                       + QStringLiteral(" x ")
                       + QString::number(committedSelection.height());
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const int predictedExtra = qMax(0, predictedSelection.height() - committedSelection.height());
    if (m_longCaptureEnabled && predictedExtra > 0)
    {
        sizeText += QStringLiteral("  预测 +%1 px").arg(predictedExtra);
    }
#endif

    int textY = committedSelection.top() - 28;

    const QRect screenRect = screenLocalRectForSelection(committedSelection);
    const int minTop = screenRect.top() + 8;
    const int maxBottom = screenRect.bottom() - 8;

    if (textY < minTop)
    {
        textY = committedSelection.bottom() + 8;
    }

    if (textY + 22 > maxBottom)
    {
        textY = qMax(minTop, maxBottom - 22);
    }

    int textX = committedSelection.left();
    const int textWidth = sizeText.contains(QStringLiteral("预测")) ? 220 : 150;
    if (textX + textWidth > screenRect.right() - 4)
    {
        textX = qMax(screenRect.left() + 4, screenRect.right() - textWidth - 4);
    }

    const QRect textRect(textX, textY, textWidth, 22);
    painter.fillRect(textRect, QColor(12, 19, 30, 220));
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, sizeText);
}

void SelectionOverlay::contextMenuEvent(QContextMenuEvent *event)
{
    if (event != nullptr)
    {
        event->accept();
    }
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void SelectionOverlay::wheelEvent(QWheelEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (m_longCaptureEnabled && m_hasSelection)
    {
        int delta = event->angleDelta().y();
        if (delta == 0)
        {
            delta = event->pixelDelta().y() * 8;
        }

        if (delta != 0)
        {
            emit longCaptureWheel(m_selectedRect, delta);
            event->accept();
            return;
        }
    }

    QWidget::wheelEvent(event);
}
#endif

bool SelectionOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_textEditor && event != nullptr)
    {
        if (event->type() == QEvent::ShortcutOverride)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                keyEvent->accept();
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape)
            {
                finishCurrentTextAnnotation(false);
                setFocus();
                return true;
            }

            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                finishCurrentTextAnnotation(true);
                setFocus();
                return true;
            }
        }
        else if (event->type() == QEvent::FocusOut)
        {
            finishCurrentTextAnnotation(true);
        }
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    if (event != nullptr && event->type() == QEvent::Wheel && m_longCaptureEnabled && m_hasSelection)
    {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        int delta = wheelEvent->angleDelta().y();
        if (delta == 0)
        {
            delta = wheelEvent->pixelDelta().y() * 8;
        }

        if (delta != 0)
        {
            emit longCaptureWheel(m_selectedRect, delta);
            wheelEvent->accept();
            return true;
        }
    }
#endif

    return QWidget::eventFilter(watched, event);
}

QRect SelectionOverlay::currentRect() const
{
    return QRect(m_startPos, m_endPos).normalized();
}

QRect SelectionOverlay::editableSelectionRect() const
{
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    return m_longCaptureEnabled ? currentDisplayRect() : currentRect();
#else
    return currentRect();
#endif
}

bool SelectionOverlay::toolSupportsStyleToolbar(ToolMode mode) const
{
    return mode == ToolMode::Pen
           || mode == ToolMode::Rectangle
           || mode == ToolMode::Ellipse
           || mode == ToolMode::Mosaic
           || mode == ToolMode::Text;
}

bool SelectionOverlay::toolUsesColor(ToolMode mode) const
{
    return mode == ToolMode::Pen
           || mode == ToolMode::Rectangle
           || mode == ToolMode::Ellipse
           || mode == ToolMode::Text;
}

QColor SelectionOverlay::selectedColorForTool(ToolMode mode) const
{
    switch (mode)
    {
        case ToolMode::Pen:
            return m_penColor;
        case ToolMode::Rectangle:
            return m_rectangleColor;
        case ToolMode::Ellipse:
            return m_ellipseColor;
        case ToolMode::Text:
            return m_textColor;
        case ToolMode::Mosaic:
        case ToolMode::None:
        default:
            return kDefaultAnnotationColor;
    }
}

void SelectionOverlay::setSelectedColorForTool(ToolMode mode, const QColor &color)
{
    switch (mode)
    {
        case ToolMode::Pen:
            m_penColor = color;
            break;
        case ToolMode::Rectangle:
            m_rectangleColor = color;
            break;
        case ToolMode::Ellipse:
            m_ellipseColor = color;
            break;
        case ToolMode::Text:
            m_textColor = color;
            break;
        case ToolMode::Mosaic:
        case ToolMode::None:
        default:
            break;
    }
}

int SelectionOverlay::selectedThicknessIndexForTool(ToolMode mode) const
{
    switch (mode)
    {
        case ToolMode::Pen:
            return m_penThicknessIndex;
        case ToolMode::Rectangle:
            return m_rectangleThicknessIndex;
        case ToolMode::Ellipse:
            return m_ellipseThicknessIndex;
        case ToolMode::Mosaic:
            return m_mosaicThicknessIndex;
        case ToolMode::Text:
            return m_textThicknessIndex;
        case ToolMode::None:
        default:
            return 1;
    }
}

void SelectionOverlay::setSelectedThicknessIndexForTool(ToolMode mode, int index)
{
    const int clamped = qBound(0, index, 2);
    switch (mode)
    {
        case ToolMode::Pen:
            m_penThicknessIndex = clamped;
            break;
        case ToolMode::Rectangle:
            m_rectangleThicknessIndex = clamped;
            break;
        case ToolMode::Ellipse:
            m_ellipseThicknessIndex = clamped;
            break;
        case ToolMode::Mosaic:
            m_mosaicThicknessIndex = clamped;
            break;
        case ToolMode::Text:
            m_textThicknessIndex = clamped;
            break;
        case ToolMode::None:
        default:
            break;
    }
}

qreal SelectionOverlay::penWidthForIndex(int index) const
{
    return kOverlayPenWidths[qBound(0, index, 2)];
}

qreal SelectionOverlay::shapeStrokeWidthForIndex(int index) const
{
    return kOverlayShapeWidths[qBound(0, index, 2)];
}

qreal SelectionOverlay::mosaicBrushSizeForIndex(int index) const
{
    return kOverlayMosaicBrushSizes[qBound(0, index, 2)];
}

int SelectionOverlay::mosaicBlockSizeForIndex(int index) const
{
    return kOverlayMosaicBlockSizes[qBound(0, index, 2)];
}

qreal SelectionOverlay::textPixelSizeForIndex(int index) const
{
    return kOverlayTextSizes[qBound(0, index, 2)];
}

QPointF SelectionOverlay::clampPointToSelection(const QPoint &point) const
{
    const QRect selection = editableSelectionRect();
    if (!selection.isValid())
    {
        return QPointF();
    }

    return QPointF(qBound(selection.left(), point.x(), selection.right()),
                   qBound(selection.top(), point.y(), selection.bottom()));
}

bool SelectionOverlay::shouldHandleToolAt(const QPoint &point) const
{
    if (!m_hasSelection || m_selecting || m_activeTool == ToolMode::None)
    {
        return false;
    }

    return editableSelectionRect().contains(point);
}

bool SelectionOverlay::shouldHandlePenAt(const QPoint &point) const
{
    return m_activeTool == ToolMode::Pen && shouldHandleToolAt(point);
}

bool SelectionOverlay::shouldHandleRectangleAt(const QPoint &point) const
{
    return m_activeTool == ToolMode::Rectangle && shouldHandleToolAt(point);
}

bool SelectionOverlay::shouldHandleEllipseAt(const QPoint &point) const
{
    return m_activeTool == ToolMode::Ellipse && shouldHandleToolAt(point);
}

bool SelectionOverlay::shouldHandleMosaicAt(const QPoint &point) const
{
    return m_activeTool == ToolMode::Mosaic && shouldHandleToolAt(point);
}

bool SelectionOverlay::shouldHandleTextAt(const QPoint &point) const
{
    return m_activeTool == ToolMode::Text && shouldHandleToolAt(point);
}

void SelectionOverlay::beginTextEditingAt(const QPoint &point)
{
    if (!m_hasSelection)
    {
        return;
    }

    finishCurrentTextAnnotation(true);

    const QRect selection = editableSelectionRect();
    if (!selection.isValid() || !selection.contains(point))
    {
        return;
    }

    const int availableWidth = qMax(60, selection.width() - kOverlayTextEditorMargin * 2);
    const int editorWidth = qMin(kOverlayTextEditorMaxWidth,
                                 qMax(kOverlayTextEditorMinWidth, availableWidth));
    const int maxX = qMax(selection.left() + kOverlayTextEditorMargin,
                          selection.right() - editorWidth - kOverlayTextEditorMargin + 1);
    const int editorX = qBound(selection.left() + kOverlayTextEditorMargin, point.x(), maxX);

    const int maxY = qMax(selection.top() + kOverlayTextEditorMargin,
                          selection.bottom() - kOverlayTextEditorHeight - kOverlayTextEditorMargin + 1);
    const int editorY = qBound(selection.top() + kOverlayTextEditorMargin, point.y(), maxY);

    m_currentTextAnchor = QPointF(editorX, editorY);
    m_currentTextColor = m_textColor;
    m_currentTextPixelSize = textPixelSizeForIndex(m_textThicknessIndex);

    auto *editor = new QLineEdit(this);
    editor->setObjectName(QStringLiteral("overlayTextEditor"));
    editor->setFont(annotationTextFont(m_currentTextPixelSize));
    editor->setPlaceholderText(QStringLiteral("输入文字"));
    editor->setFrame(false);
    editor->setGeometry(editorX, editorY, editorWidth, kOverlayTextEditorHeight);
    editor->setTextMargins(10, 0, 10, 0);
    editor->setStyleSheet(QStringLiteral(
        "QLineEdit#overlayTextEditor {"
        "color: %1;"
        "background: rgba(14, 20, 30, 228);"
        "border: 1px solid rgba(170, 205, 255, 120);"
        "border-radius: 10px;"
        "selection-background-color: rgba(76, 166, 255, 128);"
        "padding: 0 10px;"
        "}"
        "QLineEdit#overlayTextEditor::placeholder {"
        "color: rgba(228, 239, 255, 128);"
        "}").arg(m_currentTextColor.name(QColor::HexRgb)));
    editor->installEventFilter(this);
    editor->show();
    editor->raise();
    editor->setFocus();

    m_textEditor = editor;
    update();
}

void SelectionOverlay::finishCurrentPenStroke()
{
    if (!m_currentPenStroke.isEmpty())
    {
        PenStroke stroke;
        stroke.points = m_currentPenStroke;
        stroke.color = m_currentPenColor;
        stroke.width = m_currentPenWidth;
        m_penStrokes.append(stroke);
        commitAnnotationHistory();
    }

    m_currentPenStroke.clear();
    m_currentPenColor = m_penColor;
    m_currentPenWidth = penWidthForIndex(m_penThicknessIndex);
    m_drawingPenStroke = false;
}

void SelectionOverlay::finishCurrentRectangle()
{
    if (m_currentRectangle.width() >= 2.0 && m_currentRectangle.height() >= 2.0)
    {
        RectangleAnnotation rectangle;
        rectangle.rect = m_currentRectangle.normalized();
        rectangle.color = m_currentRectangleColor;
        rectangle.strokeWidth = m_currentRectangleStrokeWidth;
        m_rectangles.append(rectangle);
        commitAnnotationHistory();
    }

    m_currentRectangle = QRectF();
    m_currentRectangleAnchor = QPointF();
    m_currentRectangleColor = m_rectangleColor;
    m_currentRectangleStrokeWidth = shapeStrokeWidthForIndex(m_rectangleThicknessIndex);
    m_drawingRectangle = false;
}

void SelectionOverlay::finishCurrentEllipse()
{
    if (m_currentEllipse.width() >= 2.0 && m_currentEllipse.height() >= 2.0)
    {
        EllipseAnnotation ellipse;
        ellipse.rect = m_currentEllipse.normalized();
        ellipse.color = m_currentEllipseColor;
        ellipse.strokeWidth = m_currentEllipseStrokeWidth;
        m_ellipses.append(ellipse);
        commitAnnotationHistory();
    }

    m_currentEllipse = QRectF();
    m_currentEllipseAnchor = QPointF();
    m_currentEllipseColor = m_ellipseColor;
    m_currentEllipseStrokeWidth = shapeStrokeWidthForIndex(m_ellipseThicknessIndex);
    m_drawingEllipse = false;
}

void SelectionOverlay::finishCurrentMosaic()
{
    if (!m_currentMosaicStroke.isEmpty())
    {
        MosaicAnnotation mosaic;
        mosaic.points = m_currentMosaicStroke;
        mosaic.brushSize = m_currentMosaicBrushSize;
        mosaic.blockSize = m_currentMosaicBlockSize;
        m_mosaics.append(mosaic);
        commitAnnotationHistory();
    }

    m_currentMosaicStroke.clear();
    m_currentMosaicBrushSize = mosaicBrushSizeForIndex(m_mosaicThicknessIndex);
    m_currentMosaicBlockSize = mosaicBlockSizeForIndex(m_mosaicThicknessIndex);
    m_drawingMosaic = false;
}

void SelectionOverlay::finishCurrentTextAnnotation(bool commit)
{
    if (m_textEditor == nullptr)
    {
        return;
    }

    QLineEdit *editor = m_textEditor;
    m_textEditor = nullptr;
    editor->removeEventFilter(this);

    const QString text = editor->text().trimmed();
    const QPointF anchor = m_currentTextAnchor;
    const QColor color = m_currentTextColor;
    const qreal fontPixelSize = m_currentTextPixelSize;

    editor->hide();
    editor->deleteLater();

    m_currentTextAnchor = QPointF();
    m_currentTextColor = m_textColor;
    m_currentTextPixelSize = textPixelSizeForIndex(m_textThicknessIndex);

    if (commit && !text.isEmpty())
    {
        TextAnnotation annotation;
        annotation.anchor = anchor;
        annotation.text = text;
        annotation.color = color;
        annotation.fontPixelSize = fontPixelSize;
        m_texts.append(annotation);
        commitAnnotationHistory();
    }

    update();
}

void SelectionOverlay::clearAnnotations()
{
    m_drawingPenStroke = false;
    m_drawingRectangle = false;
    m_drawingEllipse = false;
    m_drawingMosaic = false;
    finishCurrentTextAnnotation(false);
    m_currentPenStroke.clear();
    m_currentRectangle = QRectF();
    m_currentRectangleAnchor = QPointF();
    m_currentEllipse = QRectF();
    m_currentEllipseAnchor = QPointF();
    m_currentMosaicStroke.clear();
    m_currentTextAnchor = QPointF();
    m_currentPenColor = m_penColor;
    m_currentRectangleColor = m_rectangleColor;
    m_currentEllipseColor = m_ellipseColor;
    m_currentTextColor = m_textColor;
    m_currentPenWidth = penWidthForIndex(m_penThicknessIndex);
    m_currentRectangleStrokeWidth = shapeStrokeWidthForIndex(m_rectangleThicknessIndex);
    m_currentEllipseStrokeWidth = shapeStrokeWidthForIndex(m_ellipseThicknessIndex);
    m_currentMosaicBrushSize = mosaicBrushSizeForIndex(m_mosaicThicknessIndex);
    m_currentMosaicBlockSize = mosaicBlockSizeForIndex(m_mosaicThicknessIndex);
    m_currentTextPixelSize = textPixelSizeForIndex(m_textThicknessIndex);
    m_penStrokes.clear();
    m_rectangles.clear();
    m_ellipses.clear();
    m_mosaics.clear();
    m_texts.clear();
    resetAnnotationHistory();
}

SelectionOverlay::AnnotationSnapshot SelectionOverlay::currentAnnotationSnapshot() const
{
    AnnotationSnapshot snapshot;
    snapshot.penStrokes = m_penStrokes;
    snapshot.rectangles = m_rectangles;
    snapshot.ellipses = m_ellipses;
    snapshot.mosaics = m_mosaics;
    snapshot.texts = m_texts;
    return snapshot;
}

void SelectionOverlay::applyAnnotationSnapshot(const AnnotationSnapshot &snapshot)
{
    m_drawingPenStroke = false;
    m_drawingRectangle = false;
    m_drawingEllipse = false;
    m_drawingMosaic = false;
    m_currentPenStroke.clear();
    m_currentRectangle = QRectF();
    m_currentRectangleAnchor = QPointF();
    m_currentEllipse = QRectF();
    m_currentEllipseAnchor = QPointF();
    m_currentMosaicStroke.clear();
    m_penStrokes = snapshot.penStrokes;
    m_rectangles = snapshot.rectangles;
    m_ellipses = snapshot.ellipses;
    m_mosaics = snapshot.mosaics;
    m_texts = snapshot.texts;
    update();
}

void SelectionOverlay::resetAnnotationHistory()
{
    m_annotationHistory.clear();
    m_annotationHistory.append(currentAnnotationSnapshot());
    m_annotationHistoryIndex = 0;
    updateHistoryButtons();
}

void SelectionOverlay::commitAnnotationHistory()
{
    if (m_annotationHistoryIndex < 0)
    {
        resetAnnotationHistory();
    }

    while (m_annotationHistory.size() > m_annotationHistoryIndex + 1)
    {
        m_annotationHistory.removeLast();
    }

    m_annotationHistory.append(currentAnnotationSnapshot());
    m_annotationHistoryIndex = m_annotationHistory.size() - 1;
    updateHistoryButtons();
}

void SelectionOverlay::undoLastAnnotation()
{
    if (m_annotationHistoryIndex <= 0)
    {
        updateHistoryButtons();
        return;
    }

    --m_annotationHistoryIndex;
    applyAnnotationSnapshot(m_annotationHistory.at(m_annotationHistoryIndex));
    updateHistoryButtons();
}

void SelectionOverlay::redoLastAnnotation()
{
    if (m_annotationHistoryIndex < 0 || m_annotationHistoryIndex + 1 >= m_annotationHistory.size())
    {
        updateHistoryButtons();
        return;
    }

    ++m_annotationHistoryIndex;
    applyAnnotationSnapshot(m_annotationHistory.at(m_annotationHistoryIndex));
    updateHistoryButtons();
}

void SelectionOverlay::updateHistoryButtons()
{
    const bool canUndo = m_hasSelection && m_annotationHistoryIndex > 0;
    const bool canRedo = m_hasSelection
                         && m_annotationHistoryIndex >= 0
                         && m_annotationHistoryIndex + 1 < m_annotationHistory.size();

    if (m_btnUndo != nullptr)
    {
        m_btnUndo->setEnabled(canUndo);
    }

    if (m_btnRedo != nullptr)
    {
        m_btnRedo->setEnabled(canRedo);
    }
}

void SelectionOverlay::paintPenStrokes(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, bool includeActiveStroke) const
{
    if (painter == nullptr || targetRect.width() <= 0 || targetRect.height() <= 0 || !referenceRect.isValid())
    {
        return;
    }

    const qreal scaleX = static_cast<qreal>(targetRect.width()) / static_cast<qreal>(referenceRect.width());
    const qreal scaleY = static_cast<qreal>(targetRect.height()) / static_cast<qreal>(referenceRect.height());
    const qreal scale = (scaleX + scaleY) * 0.5;

    auto drawStroke = [painter, scaleX, scaleY, scale, referenceRect](const QVector<QPointF> &points, const QColor &color, qreal width)
    {
        if (points.isEmpty())
        {
            return;
        }

        const qreal strokeWidth = qMax<qreal>(1.4, width * scale);
        const qreal dotRadius = qMax<qreal>(1.6, strokeWidth * 0.5);

        auto mappedPoint = [scaleX, scaleY, referenceRect](const QPointF &point)
        {
            return QPointF((point.x() - referenceRect.left()) * scaleX,
                           (point.y() - referenceRect.top()) * scaleY);
        };

        if (points.size() == 1)
        {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            painter->drawEllipse(mappedPoint(points.constFirst()), dotRadius, dotRadius);
            painter->restore();
            return;
        }

        QPainterPath path(mappedPoint(points.constFirst()));
        for (int index = 1; index < points.size(); ++index)
        {
            path.lineTo(mappedPoint(points.at(index)));
        }

        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(color, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(path);
        painter->restore();
    };

    painter->save();
    painter->translate(targetRect.topLeft());

    for (const PenStroke &stroke : m_penStrokes)
    {
        drawStroke(stroke.points, stroke.color, stroke.width);
    }

    if (includeActiveStroke)
    {
        drawStroke(m_currentPenStroke, m_currentPenColor, m_currentPenWidth);
    }

    painter->restore();
}

void SelectionOverlay::paintRectangles(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, bool includeActiveRectangle) const
{
    if (painter == nullptr || targetRect.width() <= 0 || targetRect.height() <= 0 || !referenceRect.isValid())
    {
        return;
    }

    const qreal scaleX = static_cast<qreal>(targetRect.width()) / static_cast<qreal>(referenceRect.width());
    const qreal scaleY = static_cast<qreal>(targetRect.height()) / static_cast<qreal>(referenceRect.height());
    const qreal scale = (scaleX + scaleY) * 0.5;

    auto mappedPoint = [scaleX, scaleY, referenceRect](const QPointF &point)
    {
        return QPointF((point.x() - referenceRect.left()) * scaleX,
                       (point.y() - referenceRect.top()) * scaleY);
    };

    auto drawRectangle = [painter, mappedPoint, scale](const QRectF &rect, const QColor &color, qreal width, bool dashed)
    {
        if (!rect.isValid() || rect.width() < 1.0 || rect.height() < 1.0)
        {
            return;
        }

        QRectF mappedRect(mappedPoint(rect.topLeft()), mappedPoint(rect.bottomRight()));
        mappedRect = mappedRect.normalized();
        const qreal strokeWidth = qMax<qreal>(1.4, width * scale);

        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(color,
                             strokeWidth,
                             dashed ? Qt::DashLine : Qt::SolidLine,
                             Qt::RoundCap,
                             Qt::RoundJoin));
        painter->drawRoundedRect(mappedRect, 2.0, 2.0);
        painter->restore();
    };

    painter->save();
    painter->translate(targetRect.topLeft());

    for (const RectangleAnnotation &rectangle : m_rectangles)
    {
        drawRectangle(rectangle.rect, rectangle.color, rectangle.strokeWidth, false);
    }

    if (includeActiveRectangle)
    {
        drawRectangle(m_currentRectangle, m_currentRectangleColor, m_currentRectangleStrokeWidth, true);
    }

    painter->restore();
}

void SelectionOverlay::paintEllipses(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, bool includeActiveEllipse) const
{
    if (painter == nullptr || targetRect.width() <= 0 || targetRect.height() <= 0 || !referenceRect.isValid())
    {
        return;
    }

    const qreal scaleX = static_cast<qreal>(targetRect.width()) / static_cast<qreal>(referenceRect.width());
    const qreal scaleY = static_cast<qreal>(targetRect.height()) / static_cast<qreal>(referenceRect.height());
    const qreal scale = (scaleX + scaleY) * 0.5;

    auto mappedPoint = [scaleX, scaleY, referenceRect](const QPointF &point)
    {
        return QPointF((point.x() - referenceRect.left()) * scaleX,
                       (point.y() - referenceRect.top()) * scaleY);
    };

    auto drawEllipse = [painter, mappedPoint, scale](const QRectF &rect, const QColor &color, qreal width, bool dashed)
    {
        if (!rect.isValid() || rect.width() < 1.0 || rect.height() < 1.0)
        {
            return;
        }

        QRectF mappedRect(mappedPoint(rect.topLeft()), mappedPoint(rect.bottomRight()));
        mappedRect = mappedRect.normalized();
        const qreal strokeWidth = qMax<qreal>(1.4, width * scale);

        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(color,
                             strokeWidth,
                             dashed ? Qt::DashLine : Qt::SolidLine,
                             Qt::RoundCap,
                             Qt::RoundJoin));
        painter->drawEllipse(mappedRect);
        painter->restore();
    };

    painter->save();
    painter->translate(targetRect.topLeft());

    for (const EllipseAnnotation &ellipse : m_ellipses)
    {
        drawEllipse(ellipse.rect, ellipse.color, ellipse.strokeWidth, false);
    }

    if (includeActiveEllipse)
    {
        drawEllipse(m_currentEllipse, m_currentEllipseColor, m_currentEllipseStrokeWidth, true);
    }

    painter->restore();
}

void SelectionOverlay::paintMosaics(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, const QImage &sourceImage, bool includeActiveMosaic) const
{
    if (painter == nullptr || targetRect.width() <= 0 || targetRect.height() <= 0 || !referenceRect.isValid() || sourceImage.isNull())
    {
        return;
    }

    const qreal scaleX = static_cast<qreal>(targetRect.width()) / static_cast<qreal>(referenceRect.width());
    const qreal scaleY = static_cast<qreal>(targetRect.height()) / static_cast<qreal>(referenceRect.height());
    const qreal scale = (scaleX + scaleY) * 0.5;

    auto mappedPoint = [scaleX, scaleY, referenceRect](const QPointF &point)
    {
        return QPointF((point.x() - referenceRect.left()) * scaleX,
                       (point.y() - referenceRect.top()) * scaleY);
    };

    auto clipPathForStroke = [mappedPoint, scale](const QVector<QPointF> &points, qreal brushSize)
    {
        QPainterPath clipPath;
        if (points.isEmpty())
        {
            return clipPath;
        }

        const qreal scaledBrushSize = qMax<qreal>(6.0, brushSize * scale);
        if (points.size() == 1)
        {
            clipPath.addEllipse(mappedPoint(points.constFirst()), scaledBrushSize * 0.5, scaledBrushSize * 0.5);
            return clipPath;
        }

        QPainterPath strokePath(mappedPoint(points.constFirst()));
        for (int index = 1; index < points.size(); ++index)
        {
            strokePath.lineTo(mappedPoint(points.at(index)));
        }

        QPainterPathStroker stroker;
        stroker.setWidth(scaledBrushSize);
        stroker.setCapStyle(Qt::RoundCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        return stroker.createStroke(strokePath);
    };

    auto drawMosaic = [painter, targetRect, &sourceImage, scale, clipPathForStroke](const QVector<QPointF> &points,
                                                                                      qreal brushSize,
                                                                                      int blockSize,
                                                                                      bool showBorder)
    {
        if (points.isEmpty())
        {
            return;
        }

        const QPainterPath clipPath = clipPathForStroke(points, brushSize);
        const QRect sourceRect = clipPath.boundingRect().toAlignedRect().translated(targetRect.topLeft()).intersected(sourceImage.rect());
        if (!sourceRect.isValid() || sourceRect.width() < 1 || sourceRect.height() < 1)
        {
            return;
        }

        const int scaledBlockSize = qMax(4, qRound(blockSize * scale));
        const QSize reducedSize(qMax(1, sourceRect.width() / scaledBlockSize),
                                qMax(1, sourceRect.height() / scaledBlockSize));
        const QImage mosaic = sourceImage.copy(sourceRect)
                                  .scaled(reducedSize, Qt::IgnoreAspectRatio, Qt::FastTransformation)
                                  .scaled(sourceRect.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
        const QRect localRect = sourceRect.translated(-targetRect.topLeft());

        painter->save();
        painter->setClipPath(clipPath);
        painter->drawImage(localRect, mosaic);
        painter->restore();

        if (showBorder)
        {
            painter->save();
            QPen borderPen(QColor(220, 230, 255, 190), 1.0, Qt::DashLine);
            borderPen.setDashPattern({5.0, 3.0});
            painter->setPen(borderPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(clipPath);
            painter->restore();
        }
    };

    painter->save();
    painter->translate(targetRect.topLeft());

    for (const MosaicAnnotation &mosaic : m_mosaics)
    {
        drawMosaic(mosaic.points, mosaic.brushSize, mosaic.blockSize, false);
    }

    if (includeActiveMosaic)
    {
        drawMosaic(m_currentMosaicStroke, m_currentMosaicBrushSize, m_currentMosaicBlockSize, true);
    }

    painter->restore();
}

void SelectionOverlay::paintTexts(QPainter *painter, const QRect &targetRect, const QRect &referenceRect) const
{
    if (painter == nullptr || targetRect.width() <= 0 || targetRect.height() <= 0 || !referenceRect.isValid())
    {
        return;
    }

    const qreal scaleX = static_cast<qreal>(targetRect.width()) / static_cast<qreal>(referenceRect.width());
    const qreal scaleY = static_cast<qreal>(targetRect.height()) / static_cast<qreal>(referenceRect.height());
    const qreal fontScale = (scaleX + scaleY) * 0.5;

    auto mappedPoint = [scaleX, scaleY, referenceRect](const QPointF &point)
    {
        return QPointF((point.x() - referenceRect.left()) * scaleX,
                       (point.y() - referenceRect.top()) * scaleY);
    };

    painter->save();
    painter->translate(targetRect.topLeft());
    painter->setClipRect(QRect(QPoint(0, 0), targetRect.size()));
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    for (const TextAnnotation &annotation : m_texts)
    {
        if (annotation.text.isEmpty())
        {
            continue;
        }

        const QPointF topLeft = mappedPoint(annotation.anchor);
        const QFont font = annotationTextFont(annotation.fontPixelSize * fontScale);
        const QFontMetricsF metrics(font);
        const QPointF baseline(topLeft.x(), topLeft.y() + metrics.ascent());

        painter->save();
        painter->setFont(font);
        painter->setPen(QColor(0, 0, 0, 160));
        painter->drawText(baseline + QPointF(1.0, 1.0), annotation.text);
        painter->setPen(annotation.color);
        painter->drawText(baseline, annotation.text);
        painter->restore();
    }

    painter->restore();
}

void SelectionOverlay::prepareForOutputCapture()
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    hide();
}

void SelectionOverlay::updateActiveCursor()
{
    if (!m_hasSelection)
    {
        setCursor(Qt::CrossCursor);
        return;
    }

    if (m_activeTool == ToolMode::Pen)
    {
        setCursor(penToolCursor());
        return;
    }

    if (m_activeTool == ToolMode::Text)
    {
        setCursor(Qt::IBeamCursor);
        return;
    }

    setCursor(Qt::CrossCursor);
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
QRect SelectionOverlay::currentDisplayRect() const
{
    return m_longCaptureEnabled
               ? longCaptureRectForHeight(m_committedLongCaptureHeight)
               : currentRect();
}

QRect SelectionOverlay::predictedDisplayRect() const
{
    return m_longCaptureEnabled
               ? longCaptureRectForHeight(qMax(m_predictedLongCaptureHeight, m_committedLongCaptureHeight))
               : currentDisplayRect();
}

QRect SelectionOverlay::longCaptureRectForHeight(int height) const
{
    QRect selection = currentRect();
    if (!m_hasSelection)
    {
        return selection;
    }

    const int targetHeight = qMax(selection.height(), height);
    const QRect screenRect = screenLocalRectForSelection(selection);

    int anchorBottom = m_longCaptureAnchorBottomLocal;
    if (anchorBottom < 0)
    {
        anchorBottom = selection.bottom();
    }

    anchorBottom = qBound(screenRect.top(), anchorBottom, screenRect.bottom());
    const int targetTop = anchorBottom - targetHeight + 1;
    const int clampedTop = qMax(screenRect.top(), targetTop);

    selection.setTop(clampedTop);
    selection.setBottom(anchorBottom);
    return selection;
}

void SelectionOverlay::updateStatusLabel()
{
    if (m_statusLabel == nullptr)
    {
        return;
    }

    QString text = m_statusText;
    if (text.isEmpty())
    {
        text = m_hasSelection ? QStringLiteral("已选择区域") : QStringLiteral("等待选区");
    }

    const QString quality = qualityText();
    if (m_longCaptureEnabled && !quality.isEmpty())
    {
        text += QStringLiteral(" · ") + quality;
    }

    m_statusLabel->setText(text);
    m_statusLabel->adjustSize();
    if (m_hasSelection)
    {
        updateStatusPosition();
    }
    else
    {
        m_statusLabel->hide();
    }
}

QString SelectionOverlay::qualityText() const
{
    switch (m_captureQuality)
    {
        case CaptureQuality::Predicting:
            return QStringLiteral("预测中");
        case CaptureQuality::WaitingForMotion:
            return QStringLiteral("等待位移");
        case CaptureQuality::WaitingForStable:
            return QStringLiteral("等待稳定");
        case CaptureQuality::WeakMatch:
            return QStringLiteral("等待更稳定画面");
        case CaptureQuality::Confirmed:
            return QStringLiteral("已确认");
        case CaptureQuality::Idle:
        default:
            return QString();
    }
}
#endif

QRect SelectionOverlay::toGlobalRect(const QRect &rect) const
{
    return rect.translated(geometry().topLeft());
}

QRect SelectionOverlay::screenLocalRectForSelection(const QRect &selection) const
{
    const QPoint fallbackGlobalCenter = toGlobalRect(selection).center();
    QPoint globalCenter = fallbackGlobalCenter;

    if (m_selectedRect.isValid())
    {
        globalCenter = m_selectedRect.center();
    }

    QScreen *targetScreen = QGuiApplication::screenAt(globalCenter);
    if (targetScreen == nullptr)
    {
        targetScreen = QGuiApplication::screenAt(fallbackGlobalCenter);
    }

    if (targetScreen == nullptr)
    {
        targetScreen = QGuiApplication::primaryScreen();
    }

    if (targetScreen == nullptr)
    {
        return rect();
    }

    QRect localRect = targetScreen->geometry().translated(-geometry().topLeft());
    localRect = localRect.intersected(rect());

    if (!localRect.isValid() || localRect.width() < 40 || localRect.height() < 40)
    {
        return rect();
    }

    return localRect;
}

void SelectionOverlay::updateToolbarPosition()
{
    if (m_toolbar == nullptr)
    {
        return;
    }

    if (!m_hasSelection)
    {
        m_toolbar->hide();
        if (m_styleToolbar != nullptr)
        {
            m_styleToolbar->hide();
        }
        updateHistoryButtons();
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const QRect selection = predictedDisplayRect();
#else
    const QRect selection = currentRect();
#endif
    const QRect screenRect = screenLocalRectForSelection(selection);
    const int toolbarWidth = m_toolbar->sizeHint().width();
    const int toolbarHeight = m_toolbar->sizeHint().height();

    const int margin = 8;
    const int minX = screenRect.left() + margin;
    const int maxX = screenRect.right() - toolbarWidth - margin;

    int toolbarX = selection.center().x() - toolbarWidth / 2;
    toolbarX = qMax(minX, qMin(maxX, toolbarX));

    const int minY = screenRect.top() + margin;
    const int maxY = screenRect.bottom() - toolbarHeight - margin;

    int toolbarY = selection.top() - toolbarHeight - 12;
    if (toolbarY < minY)
    {
        toolbarY = selection.bottom() + 12;
    }

    if (toolbarY > maxY)
    {
        toolbarY = maxY;
    }

    if (toolbarY < minY)
    {
        toolbarY = minY;
    }

    m_toolbar->move(toolbarX, toolbarY);
    m_toolbar->show();
    m_toolbar->raise();

    rebuildStyleToolbar();
    updateStyleToolbarPosition();
    updateStatusPosition();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    updatePreviewPanelPosition();
#endif
}

void SelectionOverlay::updateStyleToolbarPosition()
{
    if (m_styleToolbar == nullptr)
    {
        return;
    }

    if (!m_hasSelection || !toolSupportsStyleToolbar(m_activeTool))
    {
        m_styleToolbar->hide();
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const QRect selection = predictedDisplayRect();
#else
    const QRect selection = currentRect();
#endif
    const QRect screenRect = screenLocalRectForSelection(selection);
    const int toolbarWidth = m_styleToolbar->width();
    const int toolbarHeight = m_styleToolbar->height();
    const int margin = 8;
    const int minX = screenRect.left() + margin;
    const int maxX = screenRect.right() - toolbarWidth - margin;

    int toolbarX = m_toolbar->geometry().center().x() - toolbarWidth / 2;
    toolbarX = qMax(minX, qMin(maxX, toolbarX));

    int toolbarY = m_toolbar->geometry().bottom() + 8;
    const int minY = screenRect.top() + margin;
    const int maxY = screenRect.bottom() - toolbarHeight - margin;
    if (toolbarY > maxY)
    {
        toolbarY = m_toolbar->y() - toolbarHeight - 8;
    }
    toolbarY = qMax(minY, qMin(maxY, toolbarY));

    m_styleToolbar->move(toolbarX, toolbarY);
    m_styleToolbar->show();
    m_styleToolbar->raise();
}

void SelectionOverlay::updateStatusPosition()
{
    if (m_statusLabel == nullptr || !m_hasSelection)
    {
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const QRect selection = predictedDisplayRect();
#else
    const QRect selection = currentRect();
#endif
    const QRect screenRect = screenLocalRectForSelection(selection);
    const QSize labelSize = m_statusLabel->sizeHint();
    const int margin = 8;
    const int minX = screenRect.left() + margin;
    const int maxX = screenRect.right() - labelSize.width() - margin;

    int labelX = selection.center().x() - labelSize.width() / 2;
    labelX = qMax(minX, qMin(maxX, labelX));

    QRect toolbarGeometry;
    if (m_toolbar != nullptr && m_toolbar->isVisible())
    {
        toolbarGeometry = m_toolbar->geometry();
        if (m_styleToolbar != nullptr && m_styleToolbar->isVisible())
        {
            toolbarGeometry = toolbarGeometry.united(m_styleToolbar->geometry());
        }
    }

    int labelY = selection.top() - labelSize.height() - 12;
    if (!toolbarGeometry.isNull())
    {
        if (toolbarGeometry.y() < selection.top())
        {
            labelY = toolbarGeometry.y() - labelSize.height() - 8;
        }
        else
        {
            labelY = toolbarGeometry.bottom() + 8;
        }
    }

    const int minY = screenRect.top() + margin;
    const int maxY = screenRect.bottom() - labelSize.height() - margin;
    if (labelY < minY)
    {
        labelY = selection.bottom() + 8;
    }
    labelY = qMax(minY, qMin(maxY, labelY));

    m_statusLabel->resize(labelSize);
    m_statusLabel->move(labelX, labelY);
    m_statusLabel->show();
    m_statusLabel->raise();
}

void SelectionOverlay::ensureToolbar()
{
    if (m_toolbar != nullptr)
    {
        return;
    }

    m_toolbar = new QFrame(this);
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_toolbar->installEventFilter(this);
#endif
    m_toolbar->setObjectName(QStringLiteral("overlayToolbar"));
    m_toolbar->setFixedHeight(48);

    auto *layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(6);

    auto createTool = [this, layout](const QString &text,
                                     const QString &iconPath,
                                     const QSize &iconSize,
                                     bool checkable = false)
    {
        auto *button = new QToolButton(m_toolbar);
        button->setCheckable(checkable);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(text);
        button->setAccessibleName(text);
        button->setMinimumSize(36, 36);

        QIcon icon(iconPath);
        if (icon.isNull() && iconPath.startsWith(QStringLiteral(":/icons/")))
        {
            const QString fallback = QStringLiteral(":/icons/icons/") + iconPath.mid(QStringLiteral(":/icons/").size());
            icon = QIcon(fallback);
        }

        if (icon.isNull())
        {
            button->setText(text);
        }
        else
        {
            button->setText(QString());
            button->setIcon(icon);
            button->setIconSize(iconSize);
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        }
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
        button->installEventFilter(this);
#endif
        layout->addWidget(button);
        return button;
    };

    m_toolGroup = new QButtonGroup(this);
    m_toolGroup->setExclusive(false);

    QToolButton *btnPen = createTool(QStringLiteral("画笔"), QStringLiteral(":/icons/overlay_pen.svg"), QSize(18, 18), true);
    QToolButton *btnRect = createTool(QStringLiteral("矩形"), QStringLiteral(":/icons/overlay_rect.svg"), QSize(18, 18), true);
    QToolButton *btnEllipse = createTool(QStringLiteral("圆形"), QStringLiteral(":/icons/overlay_ellipse.svg"), QSize(18, 18), true);
    QToolButton *btnMosaic = createTool(QStringLiteral("马赛克"), QStringLiteral(":/icons/overlay_mosaic.svg"), QSize(18, 18), true);
    QToolButton *btnText = createTool(QStringLiteral("文字"), QStringLiteral(":/icons/overlay_text.svg"), QSize(18, 18), true);

    m_toolGroup->addButton(btnPen);
    m_toolGroup->addButton(btnRect);
    m_toolGroup->addButton(btnEllipse);
    m_toolGroup->addButton(btnMosaic);
    m_toolGroup->addButton(btnText);

    auto bindToolToggle = [this](QToolButton *button, ToolMode mode)
    {
        connect(button, &QToolButton::clicked, this, [this, button, mode]()
        {
            const ToolMode previousTool = m_activeTool;

            if (button->isChecked())
            {
                for (QAbstractButton *other : m_toolGroup->buttons())
                {
                    if (other == button)
                    {
                        continue;
                    }

                    const bool blocked = other->blockSignals(true);
                    other->setChecked(false);
                    other->blockSignals(blocked);
                }
                m_activeTool = mode;
            }
            else if (m_activeTool == mode)
            {
                m_activeTool = ToolMode::None;
            }

            if (previousTool == ToolMode::Text && m_activeTool != ToolMode::Text)
            {
                finishCurrentTextAnnotation(true);
            }

            updateActiveCursor();
            updateToolbarPosition();
        });
    };

    bindToolToggle(btnPen, ToolMode::Pen);
    bindToolToggle(btnRect, ToolMode::Rectangle);
    bindToolToggle(btnEllipse, ToolMode::Ellipse);
    bindToolToggle(btnMosaic, ToolMode::Mosaic);
    bindToolToggle(btnText, ToolMode::Text);

    layout->addSpacing(4);

    m_btnUndo = createTool(QStringLiteral("撤销"), QStringLiteral(":/icons/overlay_undo.svg"), QSize(18, 18));
    m_btnRedo = createTool(QStringLiteral("重做"), QStringLiteral(":/icons/overlay_redo.svg"), QSize(18, 18));
    m_btnUndo->setEnabled(false);
    m_btnRedo->setEnabled(false);

    connect(m_btnUndo, &QToolButton::clicked, this, [this]()
    {
        undoLastAnnotation();
    });

    connect(m_btnRedo, &QToolButton::clicked, this, [this]()
    {
        redoLastAnnotation();
    });

    layout->addSpacing(8);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_btnLongCapture = createTool(QStringLiteral("长截图"), QStringLiteral(":/icons/overlay_scroll.svg"), QSize(18, 18), true);

    connect(m_btnLongCapture, &QToolButton::clicked, this, [this]()
    {
        if (!m_hasSelection)
        {
            m_btnLongCapture->setChecked(false);
            return;
        }

        m_longCaptureEnabled = m_btnLongCapture->isChecked();
        const QRect current = currentRect();

        if (m_longCaptureEnabled)
        {
            m_longCaptureAnchorBottomLocal = current.bottom();
            m_committedLongCaptureHeight = qMax(current.height(), m_committedLongCaptureHeight);
            m_predictedLongCaptureHeight = qMax(m_committedLongCaptureHeight, m_predictedLongCaptureHeight);
            m_statusText = QStringLiteral("长截图已开启");
            m_captureQuality = CaptureQuality::Confirmed;
            if (m_previewPanel != nullptr)
            {
                if (m_previewPixmap.isNull())
                {
                    m_previewPanel->clearPreview();
                }
                updatePreviewPanelPosition();
                m_previewPanel->show();
                m_previewPanel->raise();
            }
        }
        else
        {
            m_predictedLongCaptureHeight = current.height();
            m_committedLongCaptureHeight = current.height();
            m_longCaptureAnchorBottomLocal = -1;
            m_statusText = QStringLiteral("长截图已关闭");
            m_captureQuality = CaptureQuality::Idle;
            if (m_previewPanel != nullptr)
            {
                m_previewPanel->hide();
            }
        }

        updateStatusLabel();
        updateToolbarPosition();
        updateActiveCursor();
        update();
        emit longCaptureToggled(m_longCaptureEnabled, m_selectedRect);
    });
#endif

    m_btnSave = createTool(QStringLiteral("保存"), QStringLiteral(":/icons/overlay_save.svg"), QSize(18, 18));
    m_btnCancel = createTool(QStringLiteral("取消"), QStringLiteral(":/icons/overlay_close.svg"), QSize(18, 18));
    m_btnConfirm = createTool(QStringLiteral("完成"), QStringLiteral(":/icons/overlay_check.svg"), QSize(18, 18));

    connect(m_btnCancel, &QToolButton::clicked, this, [this]()
    {
        emit selectionCanceled();
        close();
    });

    connect(m_btnConfirm, &QToolButton::clicked, this, &SelectionOverlay::confirmSelection);

    connect(m_btnSave, &QToolButton::clicked, this, [this]()
    {
        if (!m_hasSelection)
        {
            return;
        }

        if (m_drawingPenStroke)
        {
            finishCurrentPenStroke();
        }
        if (m_drawingRectangle)
        {
            finishCurrentRectangle();
        }
        if (m_drawingEllipse)
        {
            finishCurrentEllipse();
        }
        if (m_drawingMosaic)
        {
            finishCurrentMosaic();
        }
        if (m_textEditor != nullptr)
        {
            finishCurrentTextAnnotation(true);
        }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
        if (m_longCaptureEnabled)
        {
            emit longCaptureSaveRequested(m_selectedRect);
            return;
        }
#endif

        prepareForOutputCapture();
        emit saveRequested(m_selectedRect);
    });

    m_toolbar->setStyleSheet(
        "QFrame#overlayToolbar {"
        "background: rgba(14, 20, 30, 216);"
        "border: 1px solid rgba(132, 180, 255, 80);"
        "border-radius: 16px;"
        "}"
        "QToolButton {"
        "color: #EAF2FF;"
        "background: transparent;"
        "border: 1px solid transparent;"
        "border-radius: 10px;"
        "padding: 0px;"
        "min-width: 36px;"
        "min-height: 36px;"
        "font: 10pt 'Microsoft YaHei UI';"
        "}"
        "QToolButton:hover {"
        "background: rgba(126, 170, 255, 35);"
        "border-color: rgba(170, 205, 255, 90);"
        "}"
        "QToolButton:checked {"
        "background: rgba(114, 166, 255, 72);"
        "border-color: rgba(176, 214, 255, 120);"
        "}"
        "QToolButton:disabled {"
        "color: rgba(234, 242, 255, 110);"
        "}");

    m_statusLabel = new QLabel(QStringLiteral("等待选区"), this);
    m_statusLabel->setObjectName(QStringLiteral("overlayStatusLabel"));
    m_statusLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_statusLabel->setStyleSheet(
        "QLabel#overlayStatusLabel {"
        "color: rgba(228, 239, 255, 220);"
        "background: rgba(14, 20, 30, 226);"
        "border: 1px solid rgba(132, 180, 255, 72);"
        "border-radius: 12px;"
        "padding: 6px 12px;"
        "font: 9pt 'Microsoft YaHei UI';"
        "font-weight: 500;"
        "}");
    m_statusLabel->adjustSize();
    m_statusLabel->hide();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_previewPanel = new LongCapturePreviewPanel(this);
    m_previewPanel->installEventFilter(this);
    m_previewPanel->hide();
#endif

    m_toolbar->adjustSize();
    m_toolbar->hide();
    ensureStyleToolbar();
}

void SelectionOverlay::ensureStyleToolbar()
{
    if (m_styleToolbar != nullptr)
    {
        return;
    }

    m_styleToolbar = new QFrame(this);
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_styleToolbar->installEventFilter(this);
#endif
    m_styleToolbar->setObjectName(QStringLiteral("overlayStyleToolbar"));
    m_styleToolbar->setFixedHeight(42);
    auto *layout = new QHBoxLayout(m_styleToolbar);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(6);
    m_styleToolbar->setStyleSheet(
        "QFrame#overlayStyleToolbar {"
        "background: rgba(10, 16, 24, 222);"
        "border: 1px solid rgba(132, 180, 255, 72);"
        "border-radius: 14px;"
        "}"
        "QToolButton {"
        "color: #EAF2FF;"
        "background: transparent;"
        "border: 1px solid rgba(170, 205, 255, 38);"
        "border-radius: 9px;"
        "min-width: 30px;"
        "min-height: 28px;"
        "padding: 0 8px;"
        "font: 9pt 'Microsoft YaHei UI';"
        "}"
        "QToolButton:hover {"
        "background: rgba(126, 170, 255, 28);"
        "border-color: rgba(170, 205, 255, 90);"
        "}"
        "QToolButton:checked {"
        "background: rgba(114, 166, 255, 62);"
        "border-color: rgba(176, 214, 255, 120);"
        "}"
        "QToolButton:disabled {"
        "border-color: rgba(255, 255, 255, 20);"
        "color: rgba(234, 242, 255, 92);"
        "}");
    m_styleToolbar->hide();
}

void SelectionOverlay::rebuildStyleToolbar()
{
    ensureStyleToolbar();
    if (m_styleToolbar == nullptr)
    {
        return;
    }

    auto *layout = qobject_cast<QHBoxLayout *>(m_styleToolbar->layout());
    if (layout == nullptr)
    {
        return;
    }

    while (QLayoutItem *item = layout->takeAt(0))
    {
        if (QWidget *widget = item->widget())
        {
            delete widget;
        }
        else if (QLayout *childLayout = item->layout())
        {
            delete childLayout;
        }
        delete item;
    }
    const auto buttonGroups = m_styleToolbar->findChildren<QButtonGroup *>(QString(), Qt::FindDirectChildrenOnly);
    for (QButtonGroup *group : buttonGroups)
    {
        delete group;
    }
    layout->invalidate();

    if (!m_hasSelection || !toolSupportsStyleToolbar(m_activeTool))
    {
        m_styleToolbar->hide();
        return;
    }

    const int selectedThickness = selectedThicknessIndexForTool(m_activeTool);
    const QList<QColor> colorPalette = {
        kDefaultAnnotationColor,
        QColor(255, 184, 0),
        QColor(42, 201, 124),
        QColor(76, 166, 255),
        QColor(255, 255, 255)
    };

    auto *thicknessGroup = new QButtonGroup(m_styleToolbar);
    thicknessGroup->setExclusive(true);
    const QStringList thicknessLabels = {
        QStringLiteral("细"),
        QStringLiteral("中"),
        QStringLiteral("粗")
    };

    for (int index = 0; index < thicknessLabels.size(); ++index)
    {
        auto *button = new QToolButton(m_styleToolbar);
        button->setCheckable(true);
        button->setChecked(index == selectedThickness);
        button->setCursor(Qt::PointingHandCursor);
        button->setText(thicknessLabels.at(index));
        button->setToolTip(QStringLiteral("%1").arg(thicknessLabels.at(index)));
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
        button->installEventFilter(this);
#endif
        thicknessGroup->addButton(button);
        layout->addWidget(button);
        connect(button, &QToolButton::clicked, this, [this, index]()
        {
            setSelectedThicknessIndexForTool(m_activeTool, index);
            if (m_activeTool == ToolMode::Text && m_textEditor != nullptr)
            {
                m_currentTextPixelSize = textPixelSizeForIndex(m_textThicknessIndex);
                m_textEditor->setFont(annotationTextFont(m_currentTextPixelSize));
            }
            update();
        });
    }

    layout->addSpacing(4);

    const bool enableColors = toolUsesColor(m_activeTool);
    auto *colorGroup = new QButtonGroup(m_styleToolbar);
    colorGroup->setExclusive(true);
    const QColor selectedColor = selectedColorForTool(m_activeTool);

    for (const QColor &color : colorPalette)
    {
        auto *button = new QToolButton(m_styleToolbar);
        button->setCheckable(true);
        button->setChecked(enableColors && color == selectedColor);
        button->setEnabled(enableColors);
        button->setCursor(Qt::PointingHandCursor);
        button->setFixedSize(24, 24);
        button->setToolTip(enableColors ? color.name(QColor::HexRgb).toUpper() : QStringLiteral("马赛克不使用颜色"));
        button->setAccessibleName(QStringLiteral("注释颜色 %1").arg(color.name(QColor::HexRgb).toUpper()));
        button->setStyleSheet(QStringLiteral(
            "QToolButton {"
            "background: %1;"
            "border: 1px solid rgba(255, 255, 255, 72);"
            "border-radius: 12px;"
            "padding: 0px;"
            "min-width: 24px;"
            "min-height: 24px;"
            "}"
            "QToolButton:hover {"
            "border-color: rgba(255, 255, 255, 180);"
            "}"
            "QToolButton:checked {"
            "border: 2px solid #F7FBFF;"
            "}"
            "QToolButton:disabled {"
            "border-color: rgba(255, 255, 255, 28);"
            "}").arg(color.name(QColor::HexRgb)));
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
        button->installEventFilter(this);
#endif
        colorGroup->addButton(button);
        layout->addWidget(button);

        if (enableColors)
        {
            connect(button, &QToolButton::clicked, this, [this, color]()
            {
                setSelectedColorForTool(m_activeTool, color);
                if (m_activeTool == ToolMode::Text && m_textEditor != nullptr)
                {
                    m_currentTextColor = color;
                    m_textEditor->setStyleSheet(QStringLiteral(
                        "QLineEdit#overlayTextEditor {"
                        "color: %1;"
                        "background: rgba(14, 20, 30, 228);"
                        "border: 1px solid rgba(170, 205, 255, 120);"
                        "border-radius: 10px;"
                        "selection-background-color: rgba(76, 166, 255, 128);"
                        "padding: 0 10px;"
                        "}"
                        "QLineEdit#overlayTextEditor::placeholder {"
                        "color: rgba(228, 239, 255, 128);"
                        "}").arg(m_currentTextColor.name(QColor::HexRgb)));
                    m_textEditor->update();
                }
                update();
            });
        }
    }

    layout->activate();
    m_styleToolbar->updateGeometry();
    const int toolbarWidth = qMax(1, layout->sizeHint().width());
    m_styleToolbar->setFixedWidth(toolbarWidth);
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void SelectionOverlay::updatePreviewPanelPosition()
{
    if (m_previewPanel == nullptr || !m_hasSelection || !m_longCaptureEnabled)
    {
        return;
    }

    const QRect selection = predictedDisplayRect();
    const QRect screenRect = screenLocalRectForSelection(selection);

    const int panelWidth = m_previewPanel->width();
    const int panelHeight = m_previewPanel->height();
    const int margin = 10;

    int panelX = selection.right() + 12;
    if (panelX + panelWidth > screenRect.right() - margin)
    {
        panelX = selection.left() - panelWidth - 12;
    }

    if (panelX < screenRect.left() + margin)
    {
        panelX = screenRect.left() + margin;
    }

    int panelY = selection.top();
    const int minY = screenRect.top() + margin;
    const int maxY = qMax(minY, screenRect.bottom() - panelHeight - margin);
    panelY = qBound(minY, panelY, maxY);

    m_previewPanel->move(panelX, panelY);
}
#endif

void SelectionOverlay::resetSelection()
{
    m_selecting = false;
    m_hasSelection = false;
    m_selectedRect = QRect();
    clearAnnotations();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_longCaptureEnabled = false;
    m_predictedLongCaptureHeight = 0;
    m_committedLongCaptureHeight = 0;
    m_longCaptureAnchorBottomLocal = -1;
    m_captureQuality = CaptureQuality::Idle;
    m_statusText.clear();
    m_previewPixmap = QPixmap();

    if (m_btnLongCapture != nullptr)
    {
        const bool blocked = m_btnLongCapture->blockSignals(true);
        m_btnLongCapture->setChecked(false);
        m_btnLongCapture->blockSignals(blocked);
    }

    if (m_previewPanel != nullptr)
    {
        m_previewPanel->clearPreview();
        m_previewPanel->hide();
    }

    updateStatusLabel();
#else
    if (m_statusLabel != nullptr)
    {
        m_statusLabel->setText(QStringLiteral("等待选区"));
    }
#endif

    if (m_toolbar != nullptr)
    {
        m_toolbar->hide();
    }
    if (m_styleToolbar != nullptr)
    {
        m_styleToolbar->hide();
    }
    if (m_statusLabel != nullptr)
    {
        m_statusLabel->hide();
    }

    updateActiveCursor();
    update();
}

void SelectionOverlay::confirmSelection()
{
    if (!m_hasSelection)
    {
        return;
    }

    if (m_drawingPenStroke)
    {
        finishCurrentPenStroke();
    }
    if (m_drawingRectangle)
    {
        finishCurrentRectangle();
    }
    if (m_drawingEllipse)
    {
        finishCurrentEllipse();
    }
    if (m_drawingMosaic)
    {
        finishCurrentMosaic();
    }
    if (m_textEditor != nullptr)
    {
        finishCurrentTextAnnotation(true);
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    if (m_longCaptureEnabled)
    {
        emit longCaptureConfirmRequested(m_selectedRect);
        return;
    }
#endif

    prepareForOutputCapture();
    emit selectionFinished(m_selectedRect);
}
