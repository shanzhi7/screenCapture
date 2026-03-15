/***********************************************************************************
*
* @file         draggablehistorythumbbutton.h
* @brief        可拖拽历史缩略图按钮：支持拖出主窗口创建贴图。
*
* @author       shanzhi
* @date         2026/03/15
* @history
***********************************************************************************/

#ifndef DRAGGABLEHISTORYTHUMBBUTTON_H
#define DRAGGABLEHISTORYTHUMBBUTTON_H

#include <QPixmap>
#include <QPoint>
#include <QToolButton>

class QMouseEvent;

class DraggableHistoryThumbButton : public QToolButton
{
    Q_OBJECT

public:
    explicit DraggableHistoryThumbButton(QWidget *parent = nullptr);

    void setDragPreviewPixmap(const QPixmap &pixmap);

signals:
    void dragPinRequested(const QPoint &globalPos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool shouldStartDrag(const QPoint &currentPos) const;
    void startDrag();

private:
    QPoint m_pressPos;
    QPixmap m_dragPreviewPixmap;
    bool m_pressActive = false;
    bool m_dragHandledOnRelease = false;
};

#endif // DRAGGABLEHISTORYTHUMBBUTTON_H