/***********************************************************************************
*
* @file         pinnedimagewindow.h
* @brief        贴图悬浮窗：以置顶轻量窗口显示截图结果。
*
* @author       shanzhi
* @date         2026/03/14
* @history
***********************************************************************************/

#ifndef PINNEDIMAGEWINDOW_H
#define PINNEDIMAGEWINDOW_H

#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QWidget>

class QFrame;
class QLabel;
class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QToolButton;
class QWheelEvent;

class PinnedImageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PinnedImageWindow(const QPixmap &pixmap, QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateDisplayedPixmap();
    void updateCloseButtonPosition();
    void repositionWithinCurrentScreen(const QPoint &preferredTopLeft);
    QRect availableGeometry() const;
    qreal initialScaleFactor() const;

private:
    QFrame *m_surface = nullptr;
    QLabel *m_imageLabel = nullptr;
    QToolButton *m_closeButton = nullptr;
    QPixmap m_originalPixmap;
    QPoint m_dragOffset;
    qreal m_scaleFactor = 1.0;
    bool m_dragging = false;
    bool m_initialPlacementPending = true;
};

#endif // PINNEDIMAGEWINDOW_H