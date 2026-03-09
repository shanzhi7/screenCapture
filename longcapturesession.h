/***********************************************************************************
*
* @file         longcapturesession.h
* @brief        长截图会话状态容器。
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
        Paused,
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
    WId overlayWinId() const;

    QPixmap previewPixmap() const;
    QPixmap resultPixmap() const;
    int visualHeight() const;

    bool autoScrollEnabled() const;
    void setAutoScrollEnabled(bool enabled);

    const QImage &lastAcceptedFrame() const;
    void updateResult(const QImage &lastAcceptedFrame, const QPixmap &preview, int visualHeight);

private:
    State m_state = State::Idle;
    QRect m_captureRect;
    WId m_overlayWinId = 0;
    QImage m_lastAcceptedFrame;
    QPixmap m_previewPixmap;
    int m_visualHeight = 0;
    bool m_autoScrollEnabled = false;
};

#endif // LONGCAPTURESESSION_H
