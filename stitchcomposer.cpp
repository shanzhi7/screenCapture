#include "stitchcomposer.h"

#include <QPainter>

namespace
{
int stitchLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}
}

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
    if (appendStrip.isNull() || appendStrip.height() <= 0 || isDuplicateAppendStrip(appendStrip))
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

bool StitchComposer::isDuplicateAppendStrip(const QImage &appendStrip) const
{
    if (appendStrip.isNull() || m_result.isNull() || appendStrip.width() != m_result.width())
    {
        return false;
    }

    const int stripHeight = appendStrip.height();
    if (stripHeight <= 0 || m_result.height() < stripHeight)
    {
        return false;
    }

    const QImage tailStrip = m_result.copy(0, m_result.height() - stripHeight, m_result.width(), stripHeight);
    if (tailStrip.isNull())
    {
        return false;
    }

    double diffSum = 0.0;
    int samples = 0;
    for (int y = 0; y < stripHeight; y += 2)
    {
        const QRgb *tailLine = reinterpret_cast<const QRgb *>(tailStrip.constScanLine(y));
        const QRgb *appendLine = reinterpret_cast<const QRgb *>(appendStrip.constScanLine(y));
        for (int x = 0; x < appendStrip.width(); x += 4)
        {
            diffSum += qAbs(stitchLuma(tailLine[x]) - stitchLuma(appendLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return false;
    }

    return (diffSum / static_cast<double>(samples)) < 1.15;
}
