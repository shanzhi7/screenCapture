#include "overlapmatcher.h"

#include <QLoggingCategory>
#include <QtMath>

#include <limits>

Q_LOGGING_CATEGORY(lcMatchLog, "lc.match")

namespace
{
int overlapLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}

QString anchorKindName(ScrollAnchorKind kind)
{
    switch (kind)
    {
    case ScrollAnchorKind::Win32ScrollInfo:
        return QStringLiteral("Win32ScrollInfo");
    case ScrollAnchorKind::UiaScrollPattern:
        return QStringLiteral("UiaScrollPattern");
    case ScrollAnchorKind::UiaRangeValue:
        return QStringLiteral("UiaRangeValue");
    case ScrollAnchorKind::VisualScrollbar:
        return QStringLiteral("VisualScrollbar");
    case ScrollAnchorKind::None:
    default:
        return QStringLiteral("None");
    }
}

QString constraintModeName(ShiftConstraintMode mode)
{
    switch (mode)
    {
    case ShiftConstraintMode::Strict:
        return QStringLiteral("Strict");
    case ShiftConstraintMode::Range:
        return QStringLiteral("Range");
    case ShiftConstraintMode::None:
    default:
        return QStringLiteral("None");
    }
}

struct OverlapCandidate
{
    int appendedHeight = 0;
    double totalScore = std::numeric_limits<double>::max();
    double leftScore = std::numeric_limits<double>::max();
    double centerScore = std::numeric_limits<double>::max();
    double rightScore = std::numeric_limits<double>::max();
    double seamScore = std::numeric_limits<double>::max();
    double duplicateScore = std::numeric_limits<double>::max();
};

bool hasAnchorConstraint(const ShiftConstraint &constraint)
{
    return constraint.valid && constraint.mode != ShiftConstraintMode::None;
}

bool isOutsideConstraintRange(int appendedHeight, const ShiftConstraint &constraint)
{
    if (!hasAnchorConstraint(constraint) || constraint.minShiftPx <= 0 || constraint.maxShiftPx <= 0)
    {
        return false;
    }

    return appendedHeight < constraint.minShiftPx || appendedHeight > constraint.maxShiftPx;
}

double preferredShiftPenalty(int appendedHeight, const ShiftConstraint &constraint)
{
    if (!hasAnchorConstraint(constraint) || constraint.preferredShiftPx <= 0)
    {
        return 0.0;
    }

    const double deviation = qAbs(appendedHeight - constraint.preferredShiftPx);
    const double softTolerance = (constraint.mode == ShiftConstraintMode::Strict)
                                     ? qMax(8.0, constraint.preferredShiftPx * 0.08)
                                     : qMax(14.0, constraint.preferredShiftPx * 0.18);
    if (deviation <= softTolerance)
    {
        return 0.0;
    }

    const double penaltyScale = (constraint.mode == ShiftConstraintMode::Strict)
                                    ? qMax(6.0, constraint.preferredShiftPx * 0.06)
                                    : qMax(10.0, constraint.preferredShiftPx * 0.12);
    return qMin((constraint.mode == ShiftConstraintMode::Strict) ? 4.8 : 3.4,
                (deviation - softTolerance) / penaltyScale);
}

double rangePenalty(int appendedHeight, const ShiftConstraint &constraint)
{
    if (!hasAnchorConstraint(constraint) || constraint.mode != ShiftConstraintMode::Range || !isOutsideConstraintRange(appendedHeight, constraint))
    {
        return 0.0;
    }

    const int deviation = (appendedHeight < constraint.minShiftPx)
                              ? (constraint.minShiftPx - appendedHeight)
                              : (appendedHeight - constraint.maxShiftPx);
    return 4.2 + static_cast<double>(deviation) / qMax(10.0, constraint.preferredShiftPx * 0.10);
}
}

OverlapMatcher::OverlapMatcher()
{
}

MatchDecision OverlapMatcher::match(const QImage &previousFrame,
                                    const QImage &currentFrame,
                                    const ShiftConstraint &constraint,
                                    const MotionAnalysis &motion) const
{
    MatchDecision decision;
    decision.reason = QStringLiteral("未命中可靠 overlap");
    decision.rejectReason = MatchRejectReason::Unknown;

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
        decision.reason = QStringLiteral("可用 overlap 窗口过小");
        decision.rejectReason = MatchRejectReason::OutOfRangeReject;
        return decision;
    }

    const bool anchorAvailable = hasAnchorConstraint(constraint);
    const int baseShift = (anchorAvailable && constraint.preferredShiftPx > 0)
                              ? constraint.preferredShiftPx
                              : (motion.estimatedShiftPx > 0 ? motion.estimatedShiftPx : motion.secondaryShiftPx);

    int searchMin = minAppend;
    int searchMax = maxAppend;
    if (anchorAvailable && constraint.mode == ShiftConstraintMode::Strict)
    {
        searchMin = qMax(minAppend, constraint.minShiftPx);
        searchMax = qMin(maxAppend, constraint.maxShiftPx);
    }
    else if (baseShift > 0)
    {
        const int window = qMax(18, baseShift / 4);
        searchMin = qMax(minAppend, baseShift - window);
        searchMax = qMin(maxAppend, baseShift + window);
    }

    double bestScore = std::numeric_limits<double>::max();
    double secondBestScore = std::numeric_limits<double>::max();
    OverlapCandidate bestCandidate;

    auto acceptLegacyFallback = [&](const QString &reason, MatchRejectReason rejectReason) -> MatchDecision
    {
        MatchDecision constrainedDecision;
        constrainedDecision.reason = reason;
        constrainedDecision.rejectReason = rejectReason;

        if (rejectReason == MatchRejectReason::DuplicateReject)
        {
            return constrainedDecision;
        }

        const MatchDecision legacyDecision = tryLegacyFallback(previous, current, constraint, motion);
        if (legacyDecision.accepted)
        {
            return legacyDecision;
        }

        return constrainedDecision;
    };

    auto evaluateCandidate = [&](int appendedHeight) -> OverlapCandidate
    {
        OverlapCandidate candidate;
        candidate.appendedHeight = appendedHeight;

        const int overlapHeight = height - appendedHeight;
        if (overlapHeight < 56)
        {
            return candidate;
        }

        if (anchorAvailable && constraint.mode == ShiftConstraintMode::Strict && isOutsideConstraintRange(appendedHeight, constraint))
        {
            return candidate;
        }

        const int seamBandTop = qMax(0, overlapHeight - 28);
        candidate.leftScore = scoreForAppend(previous,
                                             current,
                                             appendedHeight,
                                             0.04,
                                             0.30,
                                             0,
                                             overlapHeight,
                                             3,
                                             2);
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
        candidate.duplicateScore = duplicateScoreForAppend(previous, current, appendedHeight);

        const double anchorPenalty = preferredShiftPenalty(appendedHeight, constraint);
        const double outOfRangePenalty = rangePenalty(appendedHeight, constraint);
        candidate.totalScore = candidate.leftScore * 0.24
                               + candidate.centerScore * 0.34
                               + candidate.rightScore * 0.10
                               + candidate.seamScore * 0.22
                               + candidate.duplicateScore * 0.10
                               + anchorPenalty
                               + outOfRangePenalty;
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

    evaluateRange(searchMin, searchMax, 2);
    if (bestCandidate.appendedHeight > 0)
    {
        const int fineMin = qMax(minAppend, bestCandidate.appendedHeight - 6);
        const int fineMax = qMin(maxAppend, bestCandidate.appendedHeight + 6);
        evaluateRange(fineMin, fineMax, 1);
    }
    else if (searchMin != minAppend || searchMax != maxAppend)
    {
        evaluateRange(minAppend, maxAppend, 4);
    }

    if (bestCandidate.appendedHeight <= 0)
    {
        decision.rejectReason = MatchRejectReason::OutOfRangeReject;
        qCInfo(lcMatchLog).noquote() << QStringLiteral("mode=%1 source=%2 reject=no_candidate")
                                            .arg(constraintModeName(constraint.mode))
                                            .arg(anchorKindName(constraint.source));
        return acceptLegacyFallback(decision.reason, decision.rejectReason);
    }

    const double gap = qMax(0.0, secondBestScore - bestScore);
    const double normalizedScore = 1.0 - qMin(1.0, bestCandidate.totalScore / 16.0);
    const double normalizedGap = qMin(1.0, gap / 4.0);
    const double confidence = qBound(0.0, normalizedScore * 0.72 + normalizedGap * 0.28, 1.0);

    if (bestCandidate.seamScore > 11.2)
    {
        return acceptLegacyFallback(QStringLiteral("接缝区域不连续"), MatchRejectReason::SeamBreakReject);
    }

    if (bestCandidate.duplicateScore < 0.82 && bestCandidate.appendedHeight <= qMax(40, height / 8))
    {
        return acceptLegacyFallback(QStringLiteral("检测到重复条带"), MatchRejectReason::DuplicateReject);
    }

    if (bestCandidate.centerScore > 8.8 || bestCandidate.leftScore > 9.2 || bestCandidate.rightScore > 9.8)
    {
        return acceptLegacyFallback(QStringLiteral("主体区域匹配偏弱"), MatchRejectReason::WeakWinnerReject);
    }

    if (anchorAvailable)
    {
        if (isOutsideConstraintRange(bestCandidate.appendedHeight, constraint))
        {
            return acceptLegacyFallback(QStringLiteral("超出锚点位移范围"), MatchRejectReason::OutOfRangeReject);
        }

        const int hardDeviation = (constraint.mode == ShiftConstraintMode::Strict)
                                      ? qMax(16, constraint.preferredShiftPx / 5)
                                      : qMax(28, constraint.preferredShiftPx / 3);
        if (constraint.preferredShiftPx > 0
            && qAbs(bestCandidate.appendedHeight - constraint.preferredShiftPx) > hardDeviation
            && (gap < 0.90 || confidence < 0.84))
        {
            return acceptLegacyFallback(QStringLiteral("位移锚点偏离过大"), MatchRejectReason::OutOfRangeReject);
        }
    }
    else if (motion.estimatedShiftPx > 0
             && qAbs(bestCandidate.appendedHeight - motion.estimatedShiftPx) > qMax(28, motion.estimatedShiftPx / 2)
             && bestCandidate.totalScore > 8.2)
    {
        return acceptLegacyFallback(QStringLiteral("位移范围异常"), MatchRejectReason::OutOfRangeReject);
    }

    if (gap < 0.55 && bestCandidate.totalScore > 7.4)
    {
        return acceptLegacyFallback(QStringLiteral("最佳候选不够明确"), MatchRejectReason::WeakWinnerReject);
    }

    if (confidence < 0.70)
    {
        return acceptLegacyFallback(QStringLiteral("overlap 置信度不足"), MatchRejectReason::LowConfidenceReject);
    }

    decision.accepted = true;
    decision.appendedHeight = bestCandidate.appendedHeight;
    decision.confidence = confidence;
    decision.reason = anchorAvailable ? QStringLiteral("accepted_anchor") : QStringLiteral("accepted");
    decision.rejectReason = MatchRejectReason::None;

    qCInfo(lcMatchLog).noquote() << QStringLiteral("mode=%1 source=%2 accepted=%3 score=%4 conf=%5 gap=%6 left=%7 center=%8 right=%9 seam=%10 dup=%11")
                                        .arg(constraintModeName(constraint.mode))
                                        .arg(anchorKindName(constraint.source))
                                        .arg(bestCandidate.appendedHeight)
                                        .arg(bestCandidate.totalScore, 0, 'f', 3)
                                        .arg(confidence, 0, 'f', 3)
                                        .arg(gap, 0, 'f', 3)
                                        .arg(bestCandidate.leftScore, 0, 'f', 3)
                                        .arg(bestCandidate.centerScore, 0, 'f', 3)
                                        .arg(bestCandidate.rightScore, 0, 'f', 3)
                                        .arg(bestCandidate.seamScore, 0, 'f', 3)
                                        .arg(bestCandidate.duplicateScore, 0, 'f', 3);
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

double OverlapMatcher::duplicateScoreForAppend(const QImage &previousFrame,
                                               const QImage &currentFrame,
                                               int appendedHeight) const
{
    if (previousFrame.isNull() || currentFrame.isNull() || previousFrame.size() != currentFrame.size())
    {
        return std::numeric_limits<double>::max();
    }

    const int stripHeight = qMin(appendedHeight, qMax(16, currentFrame.height() / 10));
    if (stripHeight <= 0)
    {
        return std::numeric_limits<double>::max();
    }

    const int width = currentFrame.width();
    const int startY = currentFrame.height() - stripHeight;
    double diffSum = 0.0;
    int samples = 0;

    for (int y = 0; y < stripHeight; y += 2)
    {
        const QRgb *previousLine = reinterpret_cast<const QRgb *>(previousFrame.constScanLine(startY + y));
        const QRgb *currentLine = reinterpret_cast<const QRgb *>(currentFrame.constScanLine(startY + y));
        for (int x = 0; x < width; x += 4)
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

MatchDecision OverlapMatcher::tryLegacyFallback(const QImage &previousFrame,
                                                const QImage &currentFrame,
                                                const ShiftConstraint &constraint,
                                                const MotionAnalysis &motion) const
{
    MatchDecision decision;
    decision.reason = QStringLiteral("legacy fallback 未命中");
    decision.rejectReason = MatchRejectReason::Unknown;

    if (previousFrame.isNull() || currentFrame.isNull() || previousFrame.size() != currentFrame.size())
    {
        return decision;
    }

    const int height = currentFrame.height();
    const int minShift = 8;
    const int maxShift = qMin(height * 3 / 4, 420);
    if (maxShift <= minShift)
    {
        decision.rejectReason = MatchRejectReason::OutOfRangeReject;
        return decision;
    }

    const int topMargin = height / 5;
    const int bottomMargin = height / 12;
    const bool anchorAvailable = hasAnchorConstraint(constraint);
    const int baseShift = (anchorAvailable && constraint.preferredShiftPx > 0)
                              ? constraint.preferredShiftPx
                              : (motion.estimatedShiftPx > 0 ? motion.estimatedShiftPx : motion.secondaryShiftPx);

    int bestShift = 0;
    double bestScore = std::numeric_limits<double>::max();

    auto evaluateRange = [&](int rangeMin, int rangeMax, int step)
    {
        for (int shift = rangeMin; shift <= rangeMax; shift += step)
        {
            if (anchorAvailable && constraint.mode == ShiftConstraintMode::Strict && isOutsideConstraintRange(shift, constraint))
            {
                continue;
            }

            const int yStart = topMargin;
            const int yEnd = height - shift - bottomMargin;
            if (yEnd <= yStart + 20)
            {
                continue;
            }

            double diffSum = 0.0;
            int samples = 0;
            for (int y = yStart; y < yEnd; y += 4)
            {
                const int py = y + shift;
                const QRgb *previousLine = reinterpret_cast<const QRgb *>(previousFrame.constScanLine(py));
                const QRgb *currentLine = reinterpret_cast<const QRgb *>(currentFrame.constScanLine(y));
                for (int x = 0; x < currentFrame.width(); x += 8)
                {
                    diffSum += qAbs(overlapLuma(previousLine[x]) - overlapLuma(currentLine[x]));
                    ++samples;
                }
            }

            if (samples <= 0)
            {
                continue;
            }

            const double leftScore = scoreForAppend(previousFrame,
                                                    currentFrame,
                                                    shift,
                                                    0.04,
                                                    0.30,
                                                    yStart,
                                                    yEnd,
                                                    4,
                                                    3);
            const double score = (diffSum / static_cast<double>(samples)) * 0.70
                                 + leftScore * 0.30
                                 + preferredShiftPenalty(shift, constraint)
                                 + rangePenalty(shift, constraint);
            if (score < bestScore)
            {
                bestScore = score;
                bestShift = shift;
            }
        }
    };

    if (anchorAvailable && constraint.mode == ShiftConstraintMode::Strict)
    {
        evaluateRange(qMax(minShift, constraint.minShiftPx), qMin(maxShift, constraint.maxShiftPx), 1);
    }
    else if (baseShift > 0)
    {
        const int window = anchorAvailable ? qMax(18, baseShift / 3) : qMax(28, baseShift / 3);
        evaluateRange(qMax(minShift, baseShift - window), qMin(maxShift, baseShift + window), 2);
    }

    if (bestShift <= 0 || bestScore > 19.0)
    {
        evaluateRange(minShift, maxShift, 4);
    }

    if (bestShift <= 0 || bestScore > 18.8)
    {
        decision.reason = QStringLiteral("legacy fallback 分数过低");
        decision.rejectReason = MatchRejectReason::LowConfidenceReject;
        return decision;
    }

    if (anchorAvailable)
    {
        if (isOutsideConstraintRange(bestShift, constraint))
        {
            decision.reason = QStringLiteral("legacy fallback 超出锚点范围");
            decision.rejectReason = MatchRejectReason::OutOfRangeReject;
            return decision;
        }

        if (constraint.preferredShiftPx > 0
            && qAbs(bestShift - constraint.preferredShiftPx) > qMax(26, constraint.preferredShiftPx / 3)
            && bestScore > 16.0)
        {
            decision.reason = QStringLiteral("legacy fallback 偏离锚点");
            decision.rejectReason = MatchRejectReason::OutOfRangeReject;
            return decision;
        }
    }
    else if (motion.estimatedShiftPx > 0
             && qAbs(bestShift - motion.estimatedShiftPx) > qMax(42, motion.estimatedShiftPx * 2 / 3)
             && bestScore > 16.5)
    {
        decision.reason = QStringLiteral("legacy fallback 位移异常");
        decision.rejectReason = MatchRejectReason::OutOfRangeReject;
        return decision;
    }

    if (duplicateScoreForAppend(previousFrame, currentFrame, bestShift) < 0.65
        && bestShift <= qMax(36, height / 9))
    {
        decision.reason = QStringLiteral("legacy fallback 检测到重复条带");
        decision.rejectReason = MatchRejectReason::DuplicateReject;
        return decision;
    }

    decision.accepted = true;
    decision.appendedHeight = bestShift;
    decision.confidence = qBound(0.45, 1.0 - bestScore / 24.0, 0.82);
    decision.reason = anchorAvailable ? QStringLiteral("accepted_legacy_anchor") : QStringLiteral("accepted_legacy");
    decision.rejectReason = MatchRejectReason::None;
    return decision;
}