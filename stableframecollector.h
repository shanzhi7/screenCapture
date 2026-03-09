/***********************************************************************************
*
* @file         stableframecollector.h
* @brief        长截图稳定帧采样器。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef STABLEFRAMECOLLECTOR_H
#define STABLEFRAMECOLLECTOR_H

#include "longcapturetypes.h"

#include <QImage>

class StableFrameCollector
{
public:
    StableFrameCollector();

    // 用检测到位移后的第一帧启动稳定采样。
    void begin(const QImage &firstFrame);

    // 喂入后续帧，满足稳定条件后返回可拼接帧。
    StableFrameResult ingest(const QImage &frame);

    // 重置采样状态。
    void reset();

private:
    double frameDiff(const QImage &left, const QImage &right) const;

private:
    QImage m_lastFrame;
    int m_sampleCount = 0;
    int m_stableCount = 0;
};

#endif // STABLEFRAMECOLLECTOR_H
