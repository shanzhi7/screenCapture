/***********************************************************************************
*
* @file         selectionoverlay.h
* @brief        区域截图覆盖层：负责选区交互、绘制与工具条触发。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef SELECTIONOVERLAY_H
#define SELECTIONOVERLAY_H

#ifndef SCREENCAPTURE_ENABLE_LONG_CAPTURE
#define SCREENCAPTURE_ENABLE_LONG_CAPTURE 0
#endif

#include "longcapturetypes.h"

#include <QButtonGroup>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QVector>
#include <QWidget>

class QContextMenuEvent;
class QEvent;
class QFrame;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QToolButton;
class QWheelEvent;

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
class LongCapturePreviewPanel;
#endif

class SelectionOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit SelectionOverlay(QWidget *parent = nullptr);
    QRect selectedRect() const;
    QPixmap applyEditsToPixmap(const QPixmap &pixmap, const QRect &captureRect) const;

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void setLongCaptureModeEnabled(bool enabled);
    void setPredictedLongCaptureHeight(int height);
    void setCommittedLongCaptureHeight(int height);
    void setLongCapturePreview(const QPixmap &preview);
    void setStatusText(const QString &text);
    void setCaptureQuality(CaptureQuality quality);
    void setCaptureDecorationsHidden(bool hidden);
#endif

signals:
    void selectionFinished(const QRect &rect);
    void selectionCanceled();
    void saveRequested(const QRect &rect);
    void colorValueCopied(const QString &colorText);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void longCaptureToggled(bool enabled, const QRect &rect);
    void longCaptureWheel(const QRect &rect, int delta);
    void longCaptureSaveRequested(const QRect &rect);
    void longCaptureConfirmRequested(const QRect &rect);
#endif

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void wheelEvent(QWheelEvent *event) override;
#endif
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum class ToolMode
    {
        None = 0,
        Pen,
        Rectangle,
        Ellipse,
        Mosaic,
        Text
    };

    struct PenStroke
    {
        QVector<QPointF> points;
        QColor color;
    };

    struct RectangleAnnotation
    {
        QRectF rect;
        QColor color;
    };

    struct EllipseAnnotation
    {
        QRectF rect;
        QColor color;
    };

    struct TextAnnotation
    {
        QPointF anchor;
        QString text;
        QColor color;
        qreal fontPixelSize = 20.0;
    };

    QRect currentRect() const;
    QRect editableSelectionRect() const;
    QPointF clampPointToSelection(const QPoint &point) const;
    bool shouldHandleToolAt(const QPoint &point) const;
    bool shouldHandlePenAt(const QPoint &point) const;
    bool shouldHandleRectangleAt(const QPoint &point) const;
    bool shouldHandleEllipseAt(const QPoint &point) const;
    bool shouldHandleTextAt(const QPoint &point) const;
    void beginTextEditingAt(const QPoint &point);
    void finishCurrentPenStroke();
    void finishCurrentRectangle();
    void finishCurrentEllipse();
    void finishCurrentTextAnnotation(bool commit);
    void clearAnnotations();
    void paintPenStrokes(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, bool includeActiveStroke) const;
    void paintRectangles(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, bool includeActiveRectangle) const;
    void paintEllipses(QPainter *painter, const QRect &targetRect, const QRect &referenceRect, bool includeActiveEllipse) const;
    void paintTexts(QPainter *painter, const QRect &targetRect, const QRect &referenceRect) const;
    void prepareForOutputCapture();
    void updateActiveCursor();
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    QRect currentDisplayRect() const;
    QRect predictedDisplayRect() const;
    QRect longCaptureRectForHeight(int height) const;
    void updateStatusLabel();
    QString qualityText() const;
#endif
    QRect toGlobalRect(const QRect &rect) const;
    QRect screenLocalRectForSelection(const QRect &selection) const;
    void captureDesktopSnapshot();
    QString copyCurrentColorValue();

    void updateToolbarPosition();
    void updateStatusPosition();
    void ensureToolbar();
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void updatePreviewPanelPosition();
#endif
    void resetSelection();
    void confirmSelection();

private:
    bool m_selecting = false;
    bool m_hasSelection = false;

    QPoint m_startPos;
    QPoint m_endPos;
    QRect m_selectedRect;
    QPoint m_cursorPos;
    QImage m_desktopSnapshot;

    QFrame *m_toolbar = nullptr;
    QButtonGroup *m_toolGroup = nullptr;
    QLabel *m_statusLabel = nullptr;
    ToolMode m_activeTool = ToolMode::None;
    QColor m_annotationColor = QColor(255, 96, 110);
    bool m_drawingPenStroke = false;
    bool m_drawingRectangle = false;
    bool m_drawingEllipse = false;
    QVector<PenStroke> m_penStrokes;
    QVector<RectangleAnnotation> m_rectangles;
    QVector<EllipseAnnotation> m_ellipses;
    QVector<TextAnnotation> m_texts;
    QVector<QPointF> m_currentPenStroke;
    QRectF m_currentRectangle;
    QPointF m_currentRectangleAnchor;
    QRectF m_currentEllipse;
    QPointF m_currentEllipseAnchor;
    QPointF m_currentTextAnchor;
    QColor m_currentPenColor = QColor(255, 96, 110);
    QColor m_currentRectangleColor = QColor(255, 96, 110);
    QColor m_currentEllipseColor = QColor(255, 96, 110);
    QColor m_currentTextColor = QColor(255, 96, 110);
    qreal m_currentTextPixelSize = 20.0;
    QLineEdit *m_textEditor = nullptr;
    QToolButton *m_btnConfirm = nullptr;
    QToolButton *m_btnCancel = nullptr;
    QToolButton *m_btnSave = nullptr;

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    bool m_longCaptureEnabled = false;
    int m_predictedLongCaptureHeight = 0;
    int m_committedLongCaptureHeight = 0;
    int m_longCaptureAnchorBottomLocal = -1;
    bool m_captureDecorationsHidden = false;
    bool m_toolbarVisibleBeforeCapture = false;
    bool m_statusLabelVisibleBeforeCapture = false;
    bool m_previewPanelVisibleBeforeCapture = false;
    CaptureQuality m_captureQuality = CaptureQuality::Idle;
    QString m_statusText;
    QToolButton *m_btnLongCapture = nullptr;
    LongCapturePreviewPanel *m_previewPanel = nullptr;
    QPixmap m_previewPixmap;
#endif
};

#endif // SELECTIONOVERLAY_H