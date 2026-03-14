/***********************************************************************************
*
* @file         longcapturetypes.h
* @brief        长截图内部通用数据结构定义。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef LONGCAPTURETYPES_H
#define LONGCAPTURETYPES_H

#include <QImage>
#include <QMetaType>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QtGlobal>

struct ScrollDispatchResult
{
    bool dispatched = false;
    QPoint injectionPoint;
};

struct ScrollTargetContext
{
    bool valid = false;
    quintptr targetWindow = 0;
    quintptr rootWindow = 0;
    QVector<quintptr> candidateWindows;
    int targetIndex = -1;
    QPoint injectionPoint;
};

struct CaptureFrame
{
    QImage image;
    qint64 timestampMs = 0;
    quint64 serial = 0;

    bool isValid() const
    {
        return !image.isNull();
    }
};

enum class CaptureQuality
{
    Idle,
    Predicting,
    WaitingForMotion,
    WaitingForStable,
    WeakMatch,
    Confirmed
};

enum class MatchRejectReason
{
    None,
    DuplicateReject,
    SeamBreakReject,
    WeakWinnerReject,
    LowConfidenceReject,
    OutOfRangeReject,
    Unknown
};

enum class ScrollAnchorKind
{
    None,
    Win32ScrollInfo,
    UiaScrollPattern,
    UiaRangeValue,
    VisualScrollbar
};

enum class ShiftConstraintMode
{
    None,
    Range,
    Strict
};

struct MotionAnalysis
{
    bool moved = false;
    int estimatedShiftPx = 0;
    int secondaryShiftPx = 0;
    int minShiftPx = 0;
    int maxShiftPx = 0;
    double confidence = 0.0;
    QString reason;
};

struct ScrollAnchorSnapshot
{
    bool valid = false;
    ScrollAnchorKind kind = ScrollAnchorKind::None;
    quintptr sourceWindow = 0;
    double position = 0.0;
    double minimum = 0.0;
    double maximum = 0.0;
    double pageSize = 0.0;
    double viewportRatio = 0.0;
    double thumbTopRatio = 0.0;
    double thumbHeightRatio = 0.0;
    bool atTop = false;
    bool atBottom = false;
    double confidence = 0.0;
    qint64 timestampMs = 0;
};

struct ShiftConstraint
{
    bool valid = false;
    ShiftConstraintMode mode = ShiftConstraintMode::None;
    ScrollAnchorKind source = ScrollAnchorKind::None;
    int preferredShiftPx = 0;
    int minShiftPx = 0;
    int maxShiftPx = 0;
    bool anchorMoved = false;
    bool indicatesEndOfContent = false;
    bool directionConflict = false;
    double confidence = 0.0;
};

struct StableFrameResult
{
    bool valid = false;
    QImage frame;
    int stableSampleCount = 0;
    qint64 stableDurationMs = 0;
};

struct MatchDecision
{
    bool accepted = false;
    int appendedHeight = 0;
    double confidence = 0.0;
    QString reason;
    MatchRejectReason rejectReason = MatchRejectReason::Unknown;
};

Q_DECLARE_METATYPE(CaptureQuality)

#endif // LONGCAPTURETYPES_H