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
#include <QtGlobal>

struct ScrollDispatchResult
{
    bool dispatched = false;
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
