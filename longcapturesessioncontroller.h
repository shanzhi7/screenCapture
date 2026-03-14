/***********************************************************************************
*
* @file         longcapturesessioncontroller.h
* @brief        长截图会话控制器：负责 Overlay、滚动投递与拼接流水线调度。
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
#include <QQueue>
#include <QRect>
#include <QTimer>

#include <memory>

class LongCaptureBackend;
class MotionDetector;
class OverlapMatcher;
class QPixmap;
class ScrollAnchorResolver;
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
    void confirmCopy();
    void saveAs();
    void cancel();
    void detachOverlay();

    bool isActive() const;
    LongCaptureSession::State state() const;

signals:
    void previewUpdated(const QPixmap &pixmap);
    void predictedVisualHeightChanged(int height);
    void committedVisualHeightChanged(int height);
    void captureQualityChanged(CaptureQuality quality);
    void copyReady(const QPixmap &pixmap);
    void saveReady(const QPixmap &pixmap);
    void failed(const QString &message);
    void statusTextChanged(const QString &message);

private slots:
    void onObserveTimeout();

private:
    struct PendingRequest
    {
        int id = 0;
        int delta = 0;
        int predictedAppendedHeight = 0;
    };

    struct ActiveRequest
    {
        bool valid = false;
        int id = 0;
        int delta = 0;
        int predictedAppendedHeight = 0;
        int motionAttempts = 0;
        int stableAttempts = 0;
        bool motionLocked = false;
        MotionAnalysis motion;
        ScrollTargetContext targetContext;
        ScrollAnchorSnapshot beforeAnchor;
        ScrollAnchorSnapshot afterAnchor;
        ShiftConstraint shiftConstraint;
        QString lastCommitReason;
        MatchRejectReason lastCommitRejectReason = MatchRejectReason::Unknown;
    };

private:
    QPoint scrollInjectionPoint() const;
    bool ensureTargetResolved(bool forceRefresh);
    bool dispatchNextRequest();
    bool dispatchActiveRequest();
    void beginObservation();
    void finishObservation();
    void handleObservationFailure(const QString &reason,
                                  MatchRejectReason rejectReason = MatchRejectReason::Unknown);
    bool tryCommitFrame(const QImage &frame, const QString &successTextSuffix = QString());
    CaptureFrame captureObservationFrame();
    ScrollAnchorSnapshot captureAnchorSnapshot(const ScrollTargetContext &context,
                                               const QImage &frameHint,
                                               ScrollAnchorKind preferredKind = ScrollAnchorKind::None) const;
    ShiftConstraint buildShiftConstraint(const ScrollAnchorSnapshot &before,
                                         const ScrollAnchorSnapshot &after,
                                         const MotionAnalysis &motion,
                                         int fallbackShift) const;
    QPixmap buildTransientPreview(const QImage &frame,
                                  int appendedHeight,
                                  const ShiftConstraint *constraint = nullptr) const;
    QImage captureFrame();
    QImage captureFrame(const QRect &captureRect);
    void setOverlayCaptureSuppressed(bool suppressed);
    void releaseCaptureSuppression();
    void updateOverlayState();
    void resetObservationState(bool keepActiveRequest = false);
    void processPendingRequests();
    void clearEndOfContentState();
    void markEndOfContentReached();
    bool isNoAppendReason(const QString &reason) const;
    void setCaptureQuality(CaptureQuality quality);
    QString failureStatusText(const QString &reason, MatchRejectReason rejectReason) const;

private:
    LongCaptureSession m_session;
    std::unique_ptr<LongCaptureBackend> m_backend;
    std::unique_ptr<ScrollDispatcher> m_scrollDispatcher;
    std::unique_ptr<ScrollAnchorResolver> m_scrollAnchorResolver;
    std::unique_ptr<MotionDetector> m_motionDetector;
    std::unique_ptr<StableFrameCollector> m_stableCollector;
    std::unique_ptr<OverlapMatcher> m_overlapMatcher;
    std::unique_ptr<StitchComposer> m_stitchComposer;

    QPointer<SelectionOverlay> m_overlay;
    QTimer m_observeTimer;

    QQueue<PendingRequest> m_pendingRequests;
    int m_requestSerial = 0;
    int m_wheelAccumulator = 0;
    int m_consecutiveNoAppendFailures = 0;
    bool m_forceTargetRefresh = true;
    bool m_endOfContentReached = false;
    ActiveRequest m_activeRequest;
    bool m_overlayCaptureSuppressed = false;
    CaptureQuality m_captureQuality = CaptureQuality::Idle;
};

#endif // LONGCAPTURESESSIONCONTROLLER_H