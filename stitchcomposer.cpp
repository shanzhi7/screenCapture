#include "stitchcomposer.h"

#include <QPainter>

StitchComposer::StitchComposer()
{
}

void StitchComposer::reset()
{
    m_lastAcceptedFrame = QImage();
    m_result = QImage();
    m_visualHeight = 0;
}

bool StitchComposer::begin(const QImage &firstFrame)
{
    reset();
    if (firstFrame.isNull())
    {
        return false;
    }

    m_lastAcceptedFrame = firstFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    m_result = m_lastAcceptedFrame;
    m_visualHeight = m_lastAcceptedFrame.height();
    return true;
}

bool StitchComposer::append(const QImage &frame, int appendedHeight)
{
    if (m_lastAcceptedFrame.isNull() || frame.isNull() || appendedHeight <= 0)
    {
        return false;
    }

    QImage current = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (current.size() != m_lastAcceptedFrame.size())
    {
        current = current.scaled(m_lastAcceptedFrame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (appendedHeight >= current.height())
    {
        return false;
    }

    const QImage appendStrip = current.copy(0,
                                            current.height() - appendedHeight,
                                            current.width(),
                                            appendedHeight);
    if (appendStrip.isNull() || appendStrip.height() <= 0)
    {
        return false;
    }

    QImage merged(m_result.width(),
                  m_result.height() + appendStrip.height(),
                  QImage::Format_ARGB32_Premultiplied);
    merged.fill(Qt::transparent);

    QPainter painter(&merged);
    painter.drawImage(0, 0, m_result);
    painter.drawImage(0, m_result.height(), appendStrip);
    painter.end();

    m_result = merged;
    m_lastAcceptedFrame = current;
    m_visualHeight += appendStrip.height();
    return true;
}

bool StitchComposer::hasResult() const
{
    return !m_result.isNull();
}

int StitchComposer::visualHeight() const
{
    return m_visualHeight;
}

QPixmap StitchComposer::resultPixmap() const
{
    if (m_result.isNull())
    {
        return QPixmap();
    }

    return QPixmap::fromImage(m_result);
}

const QImage &StitchComposer::lastAcceptedFrame() const
{
    return m_lastAcceptedFrame;
}
