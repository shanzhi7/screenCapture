#ifndef LONGCAPTURESTITCHER_H
#define LONGCAPTURESTITCHER_H

#include <QImage>
#include <QPixmap>

class LongCaptureStitcher
{
public:
    LongCaptureStitcher();

    void reset();
    bool begin(const QImage &firstFrame);

    // Append next frame and return appended pixel height.
    int append(const QImage &nextFrame);

    bool hasResult() const;
    int visualHeight() const;
    QPixmap resultPixmap() const;

private:
    bool hasMeaningfulChange(const QImage &prev, const QImage &curr) const;
    int estimateScrollShift(const QImage &prev, const QImage &curr) const;

private:
    QImage m_lastFrame;
    QImage m_merged;
    int m_visualHeight = 0;
};

#endif // LONGCAPTURESTITCHER_H
