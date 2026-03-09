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

    MatchDecision match(const QImage &previousFrame,
                        const QImage &currentFrame,
                        int expectedShiftPx,
                        int preferredShiftPx) const;

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
    double duplicateScoreForAppend(const QImage &previousFrame,
                                   const QImage &currentFrame,
                                   int appendedHeight) const;
    MatchDecision tryLegacyFallback(const QImage &previousFrame,
                                    const QImage &currentFrame,
                                    int expectedShiftPx,
                                    int preferredShiftPx) const;
};

#endif // OVERLAPMATCHER_H

