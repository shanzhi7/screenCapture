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

    void begin(const CaptureFrame &firstFrame);
    StableFrameResult ingest(const CaptureFrame &frame);
    void reset();

private:
    double frameDiff(const QImage &left, const QImage &right) const;

private:
    QImage m_lastFrame;
    int m_sampleCount = 0;
    int m_stableCount = 0;
    qint64 m_firstTimestampMs = 0;
    qint64 m_lastTimestampMs = 0;
    qint64 m_stableStartTimestampMs = 0;
};

#endif // STABLEFRAMECOLLECTOR_H
