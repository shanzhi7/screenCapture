/***********************************************************************************
*
* @file         longcapturesessioncontroller.h
* @brief        长截图会话控制器：负责 Overlay 接入、流水线编排与结果回传。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef LONGCAPTURESESSIONCONTROLLER_H
#define LONGCAPTURESESSIONCONTROLLER_H

#include "longcapturesession.h"
#include "longcapturetypes.h"

#include <QObject>
#include <QPointer>
#include <QRect>
#include <QTimer>

#include <memory>

class LongCaptureBackend;
class MotionDetector;
class OverlapMatcher;
class QPixmap;
class ScrollDispatcher;
class SelectionOverlay;
class StableFrameCollector;
class StitchComposer;

class LongCaptureSessionController : public QObject
{
    Q_OBJECT

public:
    explicit LongCaptureSessionController(QObject *parent = nullptr);
    ~LongCaptureSessionController() override;

    bool start(const QRect &selectedRect, WId overlayWinId);
    void requestManualScroll(int delta);
    void setAutoScrollEnabled(bool enabled);
    void confirmCopy();
    void saveAs();
    void cancel();

    bool isActive() const;
    LongCaptureSession::State state() const;

signals:
    void previewUpdated(const QPixmap &pixmap);
    void visualHeightChanged(int height);
    void copyReady(const QPixmap &pixmap);
    void saveReady(const QPixmap &pixmap);
    void failed(const QString &message);
    void autoPaused(const QString &reason);
    void statusTextChanged(const QString &message);

private slots:
    void onObserveTimeout();

private:
    struct ActiveRequest
    {
        int id = 0;
        bool fromAuto = false;
        int delta = 0;
        int motionAttempts = 0;
        int stableAttempts = 0;
        bool motionLocked = false;
        MotionAnalysis motion;
    };

private:
    QPoint scrollInjectionPoint() const;
    bool ensureTargetResolved(bool forceRefresh);
    bool dispatchNextRequest();
    void beginObservation(bool fromAuto, int delta);
    void finishObservation(bool success);
    void handleObservationFailure(const QString &reason, bool countAsAutoFailure);
    QImage captureFrame();
    QImage captureFrame(const QRect &captureRect);
    void releaseCaptureSuppression();
    void updateOverlayState();
    void resetObservationState();
    void processPendingRequests();

private:
    LongCaptureSession m_session;
    std::unique_ptr<LongCaptureBackend> m_backend;
    std::unique_ptr<ScrollDispatcher> m_scrollDispatcher;
    std::unique_ptr<MotionDetector> m_motionDetector;
    std::unique_ptr<StableFrameCollector> m_stableCollector;
    std::unique_ptr<OverlapMatcher> m_overlapMatcher;
    std::unique_ptr<StitchComposer> m_stitchComposer;

    QPointer<SelectionOverlay> m_overlay;
    QTimer m_observeTimer;

    int m_requestSerial = 0;
    int m_wheelAccumulator = 0;
    int m_pendingManualSteps = 0;
    bool m_forceTargetRefresh = true;
    ActiveRequest m_activeRequest;
};

#endif // LONGCAPTURESESSIONCONTROLLER_H
