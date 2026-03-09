#include "overlapmatcher.h"

#include <QtMath>

#include <limits>

namespace
{
int overlapLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}

struct OverlapCandidate
{
    int appendedHeight = 0;
    double totalScore = std::numeric_limits<double>::max();
    double centerScore = std::numeric_limits<double>::max();
    double rightScore = std::numeric_limits<double>::max();
    double seamScore = std::numeric_limits<double>::max();
};
}

OverlapMatcher::OverlapMatcher()
{
}

MatchDecision OverlapMatcher::match(const QImage &previousFrame,
                                    const QImage &currentFrame,
                                    int expectedShiftPx) const
{
    MatchDecision decision;
    decision.reason = QStringLiteral("未命中可靠 overlap");

    if (previousFrame.isNull() || currentFrame.isNull())
    {
        return decision;
    }

    QImage previous = previousFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage current = currentFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (previous.size() != current.size())
    {
        current = current.scaled(previous.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const int height = current.height();
    const int minAppend = 8;
    const int maxAppend = qMin(height * 2 / 3, 420);
    if (maxAppend <= minAppend)
    {
        return decision;
    }

    int searchMin = minAppend;
    int searchMax = maxAppend;
    if (expectedShiftPx > 0)
    {
        const int window = qMax(20, expectedShiftPx / 3);
        searchMin = qMax(minAppend, expectedShiftPx - window);
        searchMax = qMin(maxAppend, expectedShiftPx + window);
    }

    double bestScore = std::numeric_limits<double>::max();
    double secondBestScore = std::numeric_limits<double>::max();
    OverlapCandidate bestCandidate;

    auto evaluateCandidate = [&](int appendedHeight) -> OverlapCandidate
    {
        OverlapCandidate candidate;
        candidate.appendedHeight = appendedHeight;

        const int overlapHeight = height - appendedHeight;
        if (overlapHeight < 56)
        {
            return candidate;
        }

        const int seamBandTop = qMax(0, overlapHeight - 28);
        candidate.centerScore = scoreForAppend(previous,
                                               current,
                                               appendedHeight,
                                               0.12,
                                               0.88,
                                               0,
                                               overlapHeight,
                                               4,
                                               2);
        candidate.rightScore = scoreForAppend(previous,
                                              current,
                                              appendedHeight,
                                              0.66,
                                              0.90,
                                              0,
                                              overlapHeight,
                                              4,
                                              2);
        candidate.seamScore = scoreForAppend(previous,
                                             current,
                                             appendedHeight,
                                             0.16,
                                             0.84,
                                             seamBandTop,
                                             overlapHeight,
                                             3,
                                             1);
        candidate.totalScore = candidate.centerScore * 0.58
                               + candidate.rightScore * 0.16
                               + candidate.seamScore * 0.26;
        return candidate;
    };

    auto evaluateRange = [&](int rangeMin, int rangeMax, int step)
    {
        for (int appendedHeight = rangeMin; appendedHeight <= rangeMax; appendedHeight += step)
        {
            const OverlapCandidate candidate = evaluateCandidate(appendedHeight);
            if (!qIsFinite(candidate.totalScore))
            {
                continue;
            }

            if (candidate.totalScore < bestScore)
            {
                secondBestScore = bestScore;
                bestScore = candidate.totalScore;
                bestCandidate = candidate;
            }
            else if (candidate.totalScore < secondBestScore)
            {
                secondBestScore = candidate.totalScore;
            }
        }
    };

    evaluateRange(searchMin, searchMax, 4);
    if (bestCandidate.appendedHeight > 0)
    {
        const int fineMin = qMax(minAppend, bestCandidate.appendedHeight - 6);
        const int fineMax = qMin(maxAppend, bestCandidate.appendedHeight + 6);
        evaluateRange(fineMin, fineMax, 1);
    }
    else if (searchMin != minAppend || searchMax != maxAppend)
    {
        evaluateRange(minAppend, maxAppend, 6);
        if (bestCandidate.appendedHeight > 0)
        {
            const int fineMin = qMax(minAppend, bestCandidate.appendedHeight - 6);
            const int fineMax = qMin(maxAppend, bestCandidate.appendedHeight + 6);
            evaluateRange(fineMin, fineMax, 1);
        }
    }

    if (bestCandidate.appendedHeight <= 0)
    {
        return decision;
    }

    const auto buildDecision = [&](const OverlapCandidate &candidate, double confidenceFloor, bool relaxed) -> MatchDecision
    {
        MatchDecision currentDecision;
        currentDecision.reason = QStringLiteral("未命中可靠 overlap");

        const double seamLimit = relaxed ? 12.8 : 11.4;
        const double centerLimit = relaxed ? 9.6 : 8.8;
        const double rightLimit = relaxed ? 10.6 : 9.9;

        if (candidate.seamScore > seamLimit)
        {
            currentDecision.reason = QStringLiteral("接缝区域不连续");
            return currentDecision;
        }

        if (candidate.centerScore > centerLimit || candidate.rightScore > rightLimit)
        {
            return currentDecision;
        }

        const double gap = qMax(0.0, secondBestScore - bestScore);
        const double normalizedScore = 1.0 - qMin(1.0, candidate.totalScore / 16.0);
        const double normalizedGap = qMin(1.0, gap / 4.0);
        const double confidence = qBound(0.0, normalizedScore * 0.72 + normalizedGap * 0.28, 1.0);
        if (confidence < confidenceFloor)
        {
            return currentDecision;
        }

        currentDecision.accepted = true;
        currentDecision.appendedHeight = candidate.appendedHeight;
        currentDecision.confidence = confidence;
        currentDecision.reason = QStringLiteral("accepted");
        return currentDecision;
    };

    decision = buildDecision(bestCandidate, 0.72, false);
    if (decision.accepted)
    {
        return decision;
    }

    if (expectedShiftPx > 0)
    {
        const int expectedAppend = qBound(minAppend, expectedShiftPx, maxAppend);
        const OverlapCandidate fallbackCandidate = evaluateCandidate(expectedAppend);
        if (qIsFinite(fallbackCandidate.totalScore))
        {
            decision = buildDecision(fallbackCandidate, 0.64, true);
            if (decision.accepted)
            {
                return decision;
            }
        }
    }

    return decision;
}

double OverlapMatcher::scoreForAppend(const QImage &previousFrame,
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
    if (xEnd <= xStart || yEnd <= yStart)
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
            diffSum += qAbs(overlapLuma(previousLine[x]) - overlapLuma(currentLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return std::numeric_limits<double>::max();
    }

    return diffSum / static_cast<double>(samples);
}
