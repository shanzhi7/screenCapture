#include "stableframecollector.h"

#include <QtMath>

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

void StableFrameCollector::begin(const QImage &firstFrame)
{
    reset();
    if (firstFrame.isNull())
    {
        return;
    }

    m_lastFrame = firstFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    m_sampleCount = 1;
}

StableFrameResult StableFrameCollector::ingest(const QImage &frame)
{
    StableFrameResult result;

    if (frame.isNull())
    {
        return result;
    }

    QImage current = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (!m_lastFrame.isNull() && current.size() != m_lastFrame.size())
    {
        current = current.scaled(m_lastFrame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (m_lastFrame.isNull())
    {
        begin(current);
        result.stableSampleCount = m_sampleCount;
        return result;
    }

    const double diff = frameDiff(m_lastFrame, current);
    if (diff <= 1.55)
    {
        ++m_stableCount;
    }
    else
    {
        m_stableCount = 0;
    }

    m_lastFrame = current;
    ++m_sampleCount;

    result.stableSampleCount = m_sampleCount;
    if (m_stableCount >= 2 && m_sampleCount >= 3)
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

