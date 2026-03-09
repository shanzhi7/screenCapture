#include "stableframecollector.h"

#include <QDateTime>

namespace
{
int stableFrameLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}
}

StableFrameCollector::StableFrameCollector()
{
}

void StableFrameCollector::begin(const CaptureFrame &firstFrame)
{
    reset();
    if (!firstFrame.isValid())
    {
        return;
    }

    m_lastFrame = firstFrame.image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    m_sampleCount = 1;
    m_firstTimestampMs = firstFrame.timestampMs > 0 ? firstFrame.timestampMs : QDateTime::currentMSecsSinceEpoch();
    m_lastTimestampMs = m_firstTimestampMs;
}

StableFrameResult StableFrameCollector::ingest(const CaptureFrame &frame)
{
    StableFrameResult result;

    if (!frame.isValid())
    {
        return result;
    }

    QImage current = frame.image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (!m_lastFrame.isNull() && current.size() != m_lastFrame.size())
    {
        current = current.scaled(m_lastFrame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const qint64 currentTimestamp = frame.timestampMs > 0 ? frame.timestampMs : QDateTime::currentMSecsSinceEpoch();

    if (m_lastFrame.isNull())
    {
        begin(frame);
        result.stableSampleCount = m_sampleCount;
        return result;
    }

    const double diff = frameDiff(m_lastFrame, current);
    if (diff <= 2.10)
    {
        ++m_stableCount;
        if (m_stableStartTimestampMs <= 0)
        {
            m_stableStartTimestampMs = m_lastTimestampMs;
        }
    }
    else
    {
        m_stableCount = 0;
        m_stableStartTimestampMs = 0;
    }

    m_lastFrame = current;
    m_lastTimestampMs = currentTimestamp;
    ++m_sampleCount;

    result.stableSampleCount = m_sampleCount;
    result.stableDurationMs = (m_stableStartTimestampMs > 0)
                                 ? qMax<qint64>(0, currentTimestamp - m_stableStartTimestampMs)
                                 : 0;
    if (m_stableCount >= 2 && m_sampleCount >= 3 && result.stableDurationMs >= 60)
    {
        result.valid = true;
        result.frame = m_lastFrame;
    }

    return result;
}

void StableFrameCollector::reset()
{
    m_lastFrame = QImage();
    m_sampleCount = 0;
    m_stableCount = 0;
    m_firstTimestampMs = 0;
    m_lastTimestampMs = 0;
    m_stableStartTimestampMs = 0;
}

double StableFrameCollector::frameDiff(const QImage &left, const QImage &right) const
{
    if (left.isNull() || right.isNull() || left.size() != right.size())
    {
        return 999.0;
    }

    double diffSum = 0.0;
    int samples = 0;

    for (int y = 0; y < right.height(); y += 8)
    {
        const QRgb *leftLine = reinterpret_cast<const QRgb *>(left.constScanLine(y));
        const QRgb *rightLine = reinterpret_cast<const QRgb *>(right.constScanLine(y));
        for (int x = 0; x < right.width(); x += 8)
        {
            diffSum += qAbs(stableFrameLuma(leftLine[x]) - stableFrameLuma(rightLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return 999.0;
    }

    return diffSum / static_cast<double>(samples);
}

