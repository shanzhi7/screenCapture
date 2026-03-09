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

#include <QButtonGroup>
#include <QPoint>
#include <QPixmap>
#include <QRect>
#include <QWidget>

class QContextMenuEvent;
class QEvent;
class QFrame;
class QKeyEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
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

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void setLongCaptureModeEnabled(bool enabled);
    void setLongCaptureVisualHeight(int height);
    void setLongCapturePreview(const QPixmap &preview);
    void setStatusText(const QString &text);
    void setCaptureDecorationsHidden(bool hidden);
#endif

signals:
    void selectionFinished(const QRect &rect);
    void selectionCanceled();
    void saveRequested(const QRect &rect);

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
    bool eventFilter(QObject *watched, QEvent *event) override;
#endif

private:
    QRect currentRect() const;
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    QRect currentDisplayRect() const;
#endif
    QRect toGlobalRect(const QRect &rect) const;
    QRect screenLocalRectForSelection(const QRect &selection) const;

    void updateToolbarPosition();
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

    QFrame *m_toolbar = nullptr;
    QButtonGroup *m_toolGroup = nullptr;
    QLabel *m_statusLabel = nullptr;
    QToolButton *m_btnConfirm = nullptr;
    QToolButton *m_btnCancel = nullptr;
    QToolButton *m_btnSave = nullptr;

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    bool m_longCaptureEnabled = false;
    int m_longCaptureVisualHeight = 0;
    int m_longCaptureAnchorBottomLocal = -1;
    bool m_captureDecorationsHidden = false;
    QToolButton *m_btnLongCapture = nullptr;
    LongCapturePreviewPanel *m_previewPanel = nullptr;
    QPixmap m_previewPixmap;
#endif
};

#endif // SELECTIONOVERLAY_H
