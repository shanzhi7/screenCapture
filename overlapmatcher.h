/***********************************************************************************
*
* @file         overlapmatcher.h
* @brief        长截图 overlap 匹配器。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef OVERLAPMATCHER_H
#define OVERLAPMATCHER_H

#include "longcapturetypes.h"

#include <QImage>

class OverlapMatcher
{
public:
    OverlapMatcher();

    // 依据预计位移在限定范围内搜索可靠拼接结果。
    MatchDecision match(const QImage &previousFrame,
                        const QImage &currentFrame,
                        int expectedShiftPx) const;

private:
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

#endif // OVERLAPMATCHER_H
