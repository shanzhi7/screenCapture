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
#include <QList>
#include <QPixmap>
#include <QRect>
#include <QString>
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

    bool begin(const QRect &captureRect, WId overlayWinId, const QImage &firstFrame, const QString &backendName);
    void reset();

    State state() const;
    void setState(State state);

    QRect captureRect() const;

    QPixmap previewPixmap() const;
    int committedVisualHeight() const;
    int predictedVisualHeight() const;

    const QImage &lastAcceptedFrame() const;
    void updateResult(const QImage &lastAcceptedFrame, const QPixmap &preview, int committedVisualHeight);

    void setPredictedVisualHeight(int height);
    void adjustPredictedVisualHeight(int delta);
    void alignPredictedToCommitted();

    void recordCommittedAppend(int appendedHeight);
    int predictionStepHeight() const;
    int lastStableAppendHeight() const;

    void noteFailedRequest();
    void resetFailedRequests();
    int failedRequestCount() const;

    void setLastRequestId(int requestId);
    int lastRequestId() const;

    QString backendName() const;

private:
    State m_state = State::Idle;
    QRect m_captureRect;
    QImage m_lastAcceptedFrame;
    QPixmap m_previewPixmap;
    int m_committedVisualHeight = 0;
    int m_predictedVisualHeight = 0;
    QList<int> m_recentCommittedAppends;
    int m_failedRequestCount = 0;
    int m_lastRequestId = 0;
    QString m_backendName;
};

#endif // LONGCAPTURESESSION_H
