#include "draggablehistorythumbbutton.h"

#include <QApplication>
#include <QCursor>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QWidget>

namespace
{
constexpr int kDragPreviewMaxWidth = 196;
constexpr int kDragPreviewMaxHeight = 112;
const char *kHistoryThumbMimeType = "application/x-lightshadow-history-thumb";
}

DraggableHistoryThumbButton::DraggableHistoryThumbButton(QWidget *parent)
    : QToolButton(parent)
{
}

void DraggableHistoryThumbButton::setDragPreviewPixmap(const QPixmap &pixmap)
{
    m_dragPreviewPixmap = pixmap;
}

void DraggableHistoryThumbButton::mousePressEvent(QMouseEvent *event)
{
    if (event != nullptr && event->button() == Qt::LeftButton)
    {
        m_pressPos = event->pos();
        m_pressActive = true;
        m_dragHandledOnRelease = false;
    }

    QToolButton::mousePressEvent(event);
}

void DraggableHistoryThumbButton::mouseMoveEvent(QMouseEvent *event)
{
    if (event == nullptr || !(event->buttons() & Qt::LeftButton))
    {
        QToolButton::mouseMoveEvent(event);
        return;
    }

    if (!shouldStartDrag(event->pos()))
    {
        QToolButton::mouseMoveEvent(event);
        return;
    }

    startDrag();
    event->accept();
}

void DraggableHistoryThumbButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragHandledOnRelease && event != nullptr && event->button() == Qt::LeftButton)
    {
        m_dragHandledOnRelease = false;
        m_pressActive = false;
        setDown(false);
        event->accept();
        return;
    }

    QToolButton::mouseReleaseEvent(event);
}

bool DraggableHistoryThumbButton::shouldStartDrag(const QPoint &currentPos) const
{
    return m_pressActive
           && (currentPos - m_pressPos).manhattanLength() >= QApplication::startDragDistance();
}

void DraggableHistoryThumbButton::startDrag()
{
    m_dragHandledOnRelease = true;
    m_pressActive = false;
    setDown(false);

    auto *drag = new QDrag(this);
    auto *mimeData = new QMimeData();
    mimeData->setData(QString::fromUtf8(kHistoryThumbMimeType), QByteArrayLiteral("pin"));
    drag->setMimeData(mimeData);

    if (!m_dragPreviewPixmap.isNull())
    {
        const QPixmap preview = m_dragPreviewPixmap.scaled(kDragPreviewMaxWidth,
                                                           kDragPreviewMaxHeight,
                                                           Qt::KeepAspectRatio,
                                                           Qt::SmoothTransformation);
        drag->setPixmap(preview);
        drag->setHotSpot(QPoint(preview.width() / 2, preview.height() / 2));
    }

    drag->exec(Qt::CopyAction);

    QWidget *topLevel = window();
    const QPoint releasePos = QCursor::pos();
    if (topLevel == nullptr || !topLevel->frameGeometry().contains(releasePos))
    {
        emit dragPinRequested(releasePos);
    }
}