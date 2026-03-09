#include "motiondetector.h"

#include <QtMath>

#include <limits>

namespace
{
int lumaOf(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}
}

MotionDetector::MotionDetector()
{
}

MotionAnalysis MotionDetector::analyze(const QImage &previousFrame, const QImage &currentFrame) const
{
    MotionAnalysis analysis;
    analysis.reason = QStringLiteral("未检测到滚动位移");

    if (previousFrame.isNull() || currentFrame.isNull())
    {
        return analysis;
    }

    QImage current = currentFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage previous = previousFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (current.size() != previous.size())
    {
        current = current.scaled(previous.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const double sameFrameDiff = averageSameFrameDiff(previous, current);
    if (sameFrameDiff < 1.1)
    {
        analysis.reason = QStringLiteral("画面变化过小");
        return analysis;
    }

    const int height = current.height();
    const int minAppend = 4;
    const int maxAppend = qMin(height * 2 / 3, 420);
    if (maxAppend <= minAppend)
    {
        analysis.reason = QStringLiteral("可用位移窗口过小");
        return analysis;
    }

    double bestScore = std::numeric_limits<double>::max();
    double secondBestScore = std::numeric_limits<double>::max();
    int bestAppend = 0;
    int secondBestAppend = 0;

    const int topIgnore = height / 10;
    const int bottomIgnore = height / 14;

    for (int appendedHeight = minAppend; appendedHeight <= maxAppend; ++appendedHeight)
    {
        const int overlapHeight = height - appendedHeight;
        if (overlapHeight < 48)
        {
            continue;
        }

        const double centerScore = scoreForAppend(previous,
                                                  current,
                                                  appendedHeight,
                                                  0.15,
                                                  0.85,
                                                  topIgnore,
                                                  overlapHeight - bottomIgnore,
                                                  5,
                                                  3);
        const double rightScore = scoreForAppend(previous,
                                                 current,
                                                 appendedHeight,
                                                 0.60,
                                                 0.88,
                                                 topIgnore,
                                                 overlapHeight - bottomIgnore,
                                                 5,
                                                 3);

        const double totalScore = centerScore * 0.68 + rightScore * 0.32;
        if (totalScore < bestScore)
        {
            secondBestScore = bestScore;
            secondBestAppend = bestAppend;
            bestScore = totalScore;
            bestAppend = appendedHeight;
        }
        else if (totalScore < secondBestScore)
        {
            secondBestScore = totalScore;
            secondBestAppend = appendedHeight;
        }
    }

    if (bestAppend <= 0)
    {
        analysis.reason = QStringLiteral("未找到可靠位移");
        return analysis;
    }

    const double gap = qMax(0.0, secondBestScore - bestScore);
    const double normalizedScore = 1.0 - qMin(1.0, bestScore / 22.0);
    const double normalizedGap = qMin(1.0, gap / 5.0);
    const double confidence = qBound(0.0, normalizedScore * 0.72 + normalizedGap * 0.28, 1.0);

    analysis.estimatedShiftPx = bestAppend;
    analysis.secondaryShiftPx = secondBestAppend;
    analysis.confidence = confidence;

    int minShift = qMax(minAppend, bestAppend - qMax(12, bestAppend / 6));
    int maxShift = qMin(maxAppend, bestAppend + qMax(12, bestAppend / 6));
    if (secondBestAppend > 0)
    {
        const int left = qMin(bestAppend, secondBestAppend);
        const int right = qMax(bestAppend, secondBestAppend);
        const int padding = qMax(10, qAbs(bestAppend - secondBestAppend) / 2);
        minShift = qMax(minAppend, left - padding);
        maxShift = qMin(maxAppend, right + padding);
    }
    analysis.minShiftPx = minShift;
    analysis.maxShiftPx = maxShift;

    if (confidence < 0.52)
    {
        analysis.reason = QStringLiteral("位移置信度不足");
        return analysis;
    }

    analysis.moved = true;
    analysis.reason = QStringLiteral("位移确认");
    return analysis;
}

double MotionDetector::averageSameFrameDiff(const QImage &previousFrame, const QImage &currentFrame) const
{
    if (previousFrame.isNull() || currentFrame.isNull() || previousFrame.size() != currentFrame.size())
    {
        return 0.0;
    }

    double diffSum = 0.0;
    int samples = 0;

    for (int y = 0; y < currentFrame.height(); y += 8)
    {
        const QRgb *previousLine = reinterpret_cast<const QRgb *>(previousFrame.constScanLine(y));
        const QRgb *currentLine = reinterpret_cast<const QRgb *>(currentFrame.constScanLine(y));
        for (int x = 0; x < currentFrame.width(); x += 8)
        {
            diffSum += qAbs(lumaOf(previousLine[x]) - lumaOf(currentLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return 0.0;
    }

    return diffSum / static_cast<double>(samples);
}

double MotionDetector::scoreForAppend(const QImage &previousFrame,
                                      const QImage &currentFrame,
                                      int appendedHeight,
                                      double xRatioStart,
                                      double xRatioEnd,
                                      int yStart,
                                      int yEnd,
                                      int stepX,
                                      int stepY) const
{
    if (previousFrame.isNull() || currentFrame.isNull() || previousFrame.size() != currentFrame.size())
    {
        return std::numeric_limits<double>::max();
    }

    const int width = previousFrame.width();
    const int xStart = qBound(0, qFloor(width * xRatioStart), width - 1);
    const int xEnd = qBound(xStart + 1, qFloor(width * xRatioEnd), width);

    if (yEnd <= yStart || xEnd <= xStart)
    {
        return std::numeric_limits<double>::max();
    }

    double diffSum = 0.0;
    int samples = 0;

    for (int y = yStart; y < yEnd; y += stepY)
    {
        const QRgb *previousLine = reinterpret_cast<const QRgb *>(previousFrame.constScanLine(y + appendedHeight));
        const QRgb *currentLine = reinterpret_cast<const QRgb *>(currentFrame.constScanLine(y));

        for (int x = xStart; x < xEnd; x += stepX)
        {
            diffSum += qAbs(lumaOf(previousLine[x]) - lumaOf(currentLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return std::numeric_limits<double>::max();
    }

    return diffSum / static_cast<double>(samples);
}

