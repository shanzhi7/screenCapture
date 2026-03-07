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

#include <QButtonGroup>
#include <QPoint>
#include <QRect>
#include <QWidget>

class QContextMenuEvent;
class QFrame;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QToolButton;
class QWheelEvent;

/**
 * @brief 截图时的全屏覆盖窗口。
 */
class SelectionOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit SelectionOverlay(QWidget *parent = nullptr);
    QRect selectedRect() const;

    // 设置长截图可视化高度（用于选区随滚动持续变高的视觉反馈）。
    void setLongCaptureVisualHeight(int height);

signals:
    void selectionFinished(const QRect &rect);
    void selectionCanceled();
    void saveRequested(const QRect &rect);

    /**
     * @brief 长截图开关变化。
     * @param enabled 是否开启长截图模式。
     * @param rect 当前选区。
     */
    void longCaptureToggled(bool enabled, const QRect &rect);

    /**
     * @brief 长截图模式下滚轮事件。
     * @param rect 当前选区。
     * @param delta 滚轮角度增量（通常 ±120）。
     */
    void longCaptureWheel(const QRect &rect, int delta);

    // 长截图模式下点击保存。
    void longCaptureSaveRequested(const QRect &rect);

    // 长截图模式下点击确认（勾）。
    void longCaptureConfirmRequested(const QRect &rect);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QRect currentRect() const;
    QRect currentDisplayRect() const;
    QRect toGlobalRect(const QRect &rect) const;
    QRect screenLocalRectForSelection(const QRect &selection) const;

    void updateToolbarPosition();
    void ensureToolbar();
    void resetSelection();
    void confirmSelection();

private:
    bool m_selecting = false;
    bool m_hasSelection = false;
    bool m_longCaptureEnabled = false;
    int m_longCaptureVisualHeight = 0;

    QPoint m_startPos;
    QPoint m_endPos;
    QRect m_selectedRect;

    QFrame *m_toolbar = nullptr;
    QButtonGroup *m_toolGroup = nullptr;
    QToolButton *m_btnLongCapture = nullptr;
    QToolButton *m_btnConfirm = nullptr;
    QToolButton *m_btnCancel = nullptr;
    QToolButton *m_btnSave = nullptr;
};

#endif // SELECTIONOVERLAY_H
