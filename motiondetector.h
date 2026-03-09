/***********************************************************************************
*
* @file         motiondetector.h
* @brief        长截图滚动位移检测器。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include "longcapturetypes.h"

#include <QImage>

class MotionDetector
{
public:
    MotionDetector();

    MotionAnalysis analyze(const QImage &previousFrame, const QImage &currentFrame) const;

private:
    double averageSameFrameDiff(const QImage &previousFrame, const QImage &currentFrame) const;
    double scoreForAppend(const QImage &previousFrame,
                          const QImage &currentFrame,
                          int appendedHeight,
                          double xRatioStart,
                          double xRatioEnd,
                          int yStart,
                          int yEnd,
                          int stepX,
                          int stepY) const;
};

#endif // MOTIONDETECTOR_H
