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
#include <QPoint>
#include <QString>

struct ScrollDispatchResult
{
    bool dispatched = false;
    QPoint injectionPoint;
};

struct MotionAnalysis
{
    bool moved = false;
    int estimatedShiftPx = 0;
    double confidence = 0.0;
};

struct StableFrameResult
{
    bool valid = false;
    QImage frame;
    int stableSampleCount = 0;
};

struct MatchDecision
{
    bool accepted = false;
    int appendedHeight = 0;
    double confidence = 0.0;
    QString reason;
};

#endif // LONGCAPTURETYPES_H
