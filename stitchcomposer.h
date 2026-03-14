/***********************************************************************************
*
* @file         stitchcomposer.h
* @brief        长截图拼接结果管理器。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef STITCHCOMPOSER_H
#define STITCHCOMPOSER_H

#include <QImage>
#include <QPixmap>

class StitchComposer
{
public:
    StitchComposer();

    void reset();
    bool begin(const QImage &firstFrame);
    bool append(const QImage &frame, int appendedHeight);

    bool hasResult() const;
    int visualHeight() const;
    int lastAppendHeight() const;
    QPixmap resultPixmap() const;
    const QImage &lastAcceptedFrame() const;

private:
    int duplicatePrefixHeight(const QImage &appendStrip) const;
    bool isDuplicateAppendStrip(const QImage &appendStrip) const;

private:
    QImage m_lastAcceptedFrame;
    QImage m_result;
    int m_visualHeight = 0;
    int m_lastAppendHeight = 0;
};

#endif // STITCHCOMPOSER_H
