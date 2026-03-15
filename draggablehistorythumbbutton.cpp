#include "draggablehistorythumbbutton.h"

#include <QApplication>
#include <QCursor>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

namespace
{
constexpr int kDragPreviewMaxWidth = 196;
constexpr int kDragPreviewMaxHeight = 112;
const char *kHistoryThumbMimeType = "application/x-lightshadow-history-thumb";

QPixmap createDragStatusCursor()
{
    const qreal devicePixelRatio = (qApp == nullptr) ? 1.0 : qApp->devicePixelRatio();
    const QSize logicalSize(24, 24);
    const QSize pixelSize(qMax(1, qRound(logicalSize.width() * devicePixelRatio)),
                          qMax(1, qRound(logicalSize.height() * devicePixelRatio)));

    QPixmap pixmap(pixelSize);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF badgeRect(2.0, 2.0, 20.0, 20.0);
    painter.setPen(QPen(QColor(255, 255, 255, 210), 1.0));
    painter.setBrush(QColor(29, 78, 216, 236));
    painter.drawEllipse(badgeRect);

    QPen plusPen(Qt::white);
    plusPen.setWidthF(2.0);
    plusPen.setCapStyle(Qt::RoundCap);
    painter.setPen(plusPen);
    painter.drawLine(QPointF(12.0, 7.2), QPointF(12.0, 16.8));
    painter.drawLine(QPointF(7.2, 12.0), QPointF(16.8, 12.0));

    return pixmap;
}
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

    const QPixmap dragStatusCursor = createDragStatusCursor();
    drag->setDragCursor(dragStatusCursor, Qt::IgnoreAction);
    drag->setDragCursor(dragStatusCursor, Qt::CopyAction);

    if (!m_dragPreviewPixmap.isNull())
    {
        const QPixmap preview = m_dragPreviewPixmap.scaled(kDragPreviewMaxWidth,
                                                           kDragPreviewMaxHeight,
                                                           Qt::KeepAspectRatio,
                                                           Qt::SmoothTransformation);
        drag->setPixmap(preview);
        drag->setHotSpot(QPoint(preview.width() / 2, preview.height() / 2));
    }

    drag->exec(Qt::CopyAction, Qt::CopyAction);

    QWidget *topLevel = window();
    const QPoint releasePos = QCursor::pos();
    if (topLevel == nullptr || !topLevel->frameGeometry().contains(releasePos))
    {
        emit dragPinRequested(releasePos);
    }
}