/***********************************************************************************
*
* @file         longcapturesession.h
* @brief        ??????????
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef LONGCAPTURESESSION_H
#define LONGCAPTURESESSION_H

#include <QImage>
#include <QPixmap>
#include <QRect>
#include <QWidget>

class LongCaptureSession
{
public:
    enum class State
    {
        Idle,
        Armed,
        Observing,
        Stitching,
        Completed,
        Canceled
    };

public:
    LongCaptureSession();

    bool begin(const QRect &captureRect, WId overlayWinId, const QImage &firstFrame);
    void reset();

    State state() const;
    void setState(State state);

    QRect captureRect() const;

    QPixmap previewPixmap() const;
    int visualHeight() const;

    const QImage &lastAcceptedFrame() const;
    void updateResult(const QImage &lastAcceptedFrame, const QPixmap &preview, int visualHeight);

private:
    State m_state = State::Idle;
    QRect m_captureRect;
    QImage m_lastAcceptedFrame;
    QPixmap m_previewPixmap;
    int m_visualHeight = 0;
};

#endif // LONGCAPTURESESSION_H
