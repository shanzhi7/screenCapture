#include "stitchcomposer.h"

#include <QLoggingCategory>
#include <QPainter>

Q_LOGGING_CATEGORY(lcStitchLog, "lc.stitch")

namespace
{
int stitchLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}

double averageStripDifference(const QImage &resultImage, int resultY, const QImage &appendStrip)
{
    if (resultImage.isNull() || appendStrip.isNull())
    {
        return 1.0e9;
    }

    if (resultY < 0 || resultY + appendStrip.height() > resultImage.height() || appendStrip.width() != resultImage.width())
    {
        return 1.0e9;
    }

    double diffSum = 0.0;
    int samples = 0;
    for (int y = 0; y < appendStrip.height(); y += 2)
    {
        const QRgb *resultLine = reinterpret_cast<const QRgb *>(resultImage.constScanLine(resultY + y));
        const QRgb *appendLine = reinterpret_cast<const QRgb *>(appendStrip.constScanLine(y));
        for (int x = 0; x < appendStrip.width(); x += 4)
        {
            diffSum += qAbs(stitchLuma(resultLine[x]) - stitchLuma(appendLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return 1.0e9;
    }

    return diffSum / static_cast<double>(samples);
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
    m_lastAppendHeight = 0;
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
    m_lastAppendHeight = 0;
    return true;
}

bool StitchComposer::append(const QImage &frame, int appendedHeight)
{
    m_lastAppendHeight = 0;
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

    QImage appendStrip = current.copy(0,
                                      current.height() - appendedHeight,
                                      current.width(),
                                      appendedHeight);
    if (appendStrip.isNull() || appendStrip.height() <= 0)
    {
        return false;
    }

    const int duplicatePrefix = duplicatePrefixHeight(appendStrip);
    if (duplicatePrefix > 0)
    {
        if (duplicatePrefix >= appendStrip.height())
        {
            qCInfo(lcStitchLog) << "reject full duplicate append strip" << duplicatePrefix;
            return false;
        }

        qCInfo(lcStitchLog) << "trim duplicate prefix" << duplicatePrefix << "from" << appendStrip.height();
        appendStrip = appendStrip.copy(0,
                                       duplicatePrefix,
                                       appendStrip.width(),
                                       appendStrip.height() - duplicatePrefix);
    }

    if (appendStrip.isNull() || appendStrip.height() <= 0 || isDuplicateAppendStrip(appendStrip))
    {
        qCInfo(lcStitchLog) << "reject append strip after duplicate screening" << appendedHeight;
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
    m_lastAppendHeight = appendStrip.height();
    qCInfo(lcStitchLog) << "append accepted" << m_lastAppendHeight << "visualHeight(before)" << m_visualHeight;
    m_visualHeight += m_lastAppendHeight;
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

int StitchComposer::lastAppendHeight() const
{
    return m_lastAppendHeight;
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

int StitchComposer::duplicatePrefixHeight(const QImage &appendStrip) const
{
    if (appendStrip.isNull() || m_result.isNull() || appendStrip.width() != m_result.width())
    {
        return 0;
    }

    const int minOverlap = 24;
    const int maxOverlap = qMin(qMin(appendStrip.height() - 8, m_result.height()), appendStrip.height() - 4);
    if (maxOverlap < minOverlap)
    {
        return 0;
    }

    for (int overlapHeight = maxOverlap; overlapHeight >= minOverlap; overlapHeight -= 4)
    {
        const QImage prefixStrip = appendStrip.copy(0, 0, appendStrip.width(), overlapHeight);
        if (prefixStrip.isNull())
        {
            continue;
        }

        const double overlapDiff = averageStripDifference(m_result,
                                                          m_result.height() - overlapHeight,
                                                          prefixStrip);
        if (overlapDiff < 1.20)
        {
            return overlapHeight;
        }
    }

    return 0;
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

    const double tailDiff = averageStripDifference(m_result,
                                                   m_result.height() - stripHeight,
                                                   appendStrip);
    if (tailDiff < 1.15)
    {
        return true;
    }

    const int recentSearchHeight = qMin(m_result.height() - stripHeight,
                                        qMax(m_lastAcceptedFrame.height() * 2,
                                             stripHeight * 4));
    if (recentSearchHeight <= 0)
    {
        return false;
    }

    const int searchStartY = qMax(0, m_result.height() - stripHeight - recentSearchHeight);
    const int searchEndY = m_result.height() - stripHeight;
    double bestRecentDiff = 1.0e9;
    for (int resultY = searchEndY - 4; resultY >= searchStartY; resultY -= 4)
    {
        bestRecentDiff = qMin(bestRecentDiff, averageStripDifference(m_result, resultY, appendStrip));
        if (bestRecentDiff < 1.40)
        {
            return true;
        }
    }

    return false;
}