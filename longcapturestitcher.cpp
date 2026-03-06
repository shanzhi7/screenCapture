#include "longcapturestitcher.h"

#include <QPainter>

#include <algorithm>`r`n#include <limits>

namespace
{
int pixelLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}
}

LongCaptureStitcher::LongCaptureStitcher()
{
}

void LongCaptureStitcher::reset()
{
    m_lastFrame = QImage();
    m_merged = QImage();
    m_visualHeight = 0;
}

bool LongCaptureStitcher::begin(const QImage &firstFrame)
{
    if (firstFrame.isNull())
    {
        return false;
    }

    m_lastFrame = firstFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    m_merged = m_lastFrame;
    m_visualHeight = m_lastFrame.height();
    return true;
}

int LongCaptureStitcher::append(const QImage &nextFrame)
{
    if (m_lastFrame.isNull() || nextFrame.isNull())
    {
        return 0;
    }

    QImage current = nextFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (current.size() != m_lastFrame.size())
    {
        current = current.scaled(m_lastFrame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (!hasMeaningfulChange(m_lastFrame, current))
    {
        return 0;
    }

    const int shift = estimateScrollShift(m_lastFrame, current);
    if (shift <= 0 || shift >= current.height())
    {
        return 0;
    }

    const QImage appendStrip = current.copy(0, current.height() - shift, current.width(), shift);
    if (appendStrip.isNull() || appendStrip.height() <= 0)
    {
        return 0;
    }

    QImage merged(m_merged.width(), m_merged.height() + appendStrip.height(), QImage::Format_ARGB32_Premultiplied);
    merged.fill(Qt::transparent);

    QPainter painter(&merged);
    painter.drawImage(0, 0, m_merged);
    painter.drawImage(0, m_merged.height(), appendStrip);
    painter.end();

    m_merged = merged;
    m_lastFrame = current;
    m_visualHeight += appendStrip.height();
    return appendStrip.height();
}

bool LongCaptureStitcher::hasResult() const
{
    return !m_merged.isNull();
}

int LongCaptureStitcher::visualHeight() const
{
    return m_visualHeight;
}

QPixmap LongCaptureStitcher::resultPixmap() const
{
    if (m_merged.isNull())
    {
        return QPixmap();
    }

    return QPixmap::fromImage(m_merged);
}

bool LongCaptureStitcher::hasMeaningfulChange(const QImage &prev, const QImage &curr) const
{
    if (prev.isNull() || curr.isNull() || prev.size() != curr.size())
    {
        return true;
    }

    const int stepX = 12;
    const int stepY = 10;

    double diffSum = 0.0;
    int samples = 0;

    for (int y = 0; y < curr.height(); y += stepY)
    {
        for (int x = 0; x < curr.width(); x += stepX)
        {
            const int l1 = pixelLuma(prev.pixel(x, y));
            const int l2 = pixelLuma(curr.pixel(x, y));
            diffSum += qAbs(l1 - l2);
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return false;
    }

    const double avgDiff = diffSum / static_cast<double>(samples);
    return avgDiff >= 2.2;
}

int LongCaptureStitcher::estimateScrollShift(const QImage &prev, const QImage &curr) const
{
    if (prev.isNull() || curr.isNull() || prev.size() != curr.size())
    {
        return 0;
    }

    const int width = curr.width();
    const int height = curr.height();

    const int minShift = 8;
    const int maxShift = std::min(height * 3 / 4, 420);
    if (maxShift <= minShift)
    {
        return 0;
    }

    // Ignore top fixed bars to reduce false match.
    const int topMargin = height / 5;
    const int bottomMargin = height / 12;

    const int stepX = 8;
    const int stepY = 4;

    int bestShift = 0;
    double bestScore = std::numeric_limits<double>::max();

    for (int shift = minShift; shift <= maxShift; ++shift)
    {
        const int yStart = topMargin;
        const int yEnd = height - shift - bottomMargin;
        if (yEnd <= yStart + 20)
        {
            continue;
        }

        double diffSum = 0.0;
        int samples = 0;

        for (int y = yStart; y < yEnd; y += stepY)
        {
            const int py = y + shift;
            for (int x = 0; x < width; x += stepX)
            {
                const int l1 = pixelLuma(prev.pixel(x, py));
                const int l2 = pixelLuma(curr.pixel(x, y));
                diffSum += qAbs(l1 - l2);
                ++samples;
            }
        }

        if (samples == 0)
        {
            continue;
        }

        const double score = diffSum / static_cast<double>(samples);
        if (score < bestScore)
        {
            bestScore = score;
            bestShift = shift;
        }
    }

    // Too different means no reliable overlap, skip this frame.
    if (bestScore > 18.5)
    {
        return 0;
    }

    return bestShift;
}

