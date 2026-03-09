#include "longcapturesessioncontroller.h"

#include "desktopgdicapturebackend.h"
#include "dxgiduplicationbackend.h"
#include "motiondetector.h"
#include "overlapmatcher.h"
#include "scrolldispatcher.h"
#include "selectionoverlay.h"
#include "showtip.h"
#include "stableframecollector.h"
#include "stitchcomposer.h"
#include "wgccapturebackend.h"

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QEventLoop>
#include <QPainter>
#include <QThread>
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
class ScopedOverlayCaptureSuppression
{
public:
    explicit ScopedOverlayCaptureSuppression(SelectionOverlay *overlay)
        : m_overlay(overlay)
        , m_active(overlay != nullptr && overlay->isVisible())
    {
        if (m_active)
        {
            m_overlay->setCaptureDecorationsHidden(true);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
        }
    }

    ~ScopedOverlayCaptureSuppression()
    {
        if (m_active)
        {
            m_overlay->setCaptureDecorationsHidden(false);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
        }
    }

private:
    SelectionOverlay *m_overlay = nullptr;
    bool m_active = false;
};

#ifdef Q_OS_WIN
class ScopedOverlayInputPassthrough
{
public:
    explicit ScopedOverlayInputPassthrough(SelectionOverlay *overlay)
        : m_overlay(overlay)
    {
        if (m_overlay == nullptr || !m_overlay->isVisible())
        {
            return;
        }

        m_window = reinterpret_cast<HWND>(m_overlay->winId());
        if (m_window == nullptr || !IsWindow(m_window))
        {
            m_window = nullptr;
            return;
        }

        m_previousExStyle = GetWindowLongPtrW(m_window, GWL_EXSTYLE);
        SetWindowLongPtrW(m_window, GWL_EXSTYLE, m_previousExStyle | WS_EX_TRANSPARENT);
        SetWindowPos(m_window,
                     nullptr,
                     0,
                     0,
                     0,
                     0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
        Sleep(6);
        m_active = true;
    }

    ~ScopedOverlayInputPassthrough()
    {
        if (!m_active || m_window == nullptr)
        {
            return;
        }

        SetWindowLongPtrW(m_window, GWL_EXSTYLE, m_previousExStyle);
        SetWindowPos(m_window,
                     nullptr,
                     0,
                     0,
                     0,
                     0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        if (m_overlay != nullptr)
        {
            m_overlay->raise();
            m_overlay->activateWindow();
            m_overlay->setFocus();
        }
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
    }

private:
    QPointer<SelectionOverlay> m_overlay;
    HWND m_window = nullptr;
    LONG_PTR m_previousExStyle = 0;
    bool m_active = false;
};
#else
class ScopedOverlayInputPassthrough
{
public:
    explicit ScopedOverlayInputPassthrough(SelectionOverlay *overlay)
    {
        Q_UNUSED(overlay)
    }
};
#endif

int backendSampleLuma(QRgb rgb)
{
    return (qRed(rgb) * 38 + qGreen(rgb) * 75 + qBlue(rgb) * 15) >> 7;
}

bool desktopFramesRoughlyMatch(const QImage &candidateFrame, const QImage &referenceFrame)
{
    if (candidateFrame.isNull() || referenceFrame.isNull() || candidateFrame.size() != referenceFrame.size())
    {
        return false;
    }

    const int stepX = qMax(1, candidateFrame.width() / 48);
    const int stepY = qMax(1, candidateFrame.height() / 48);

    double diffSum = 0.0;
    int samples = 0;
    for (int y = 0; y < candidateFrame.height(); y += stepY)
    {
        const QRgb *candidateLine = reinterpret_cast<const QRgb *>(candidateFrame.constScanLine(y));
        const QRgb *referenceLine = reinterpret_cast<const QRgb *>(referenceFrame.constScanLine(y));
        for (int x = 0; x < candidateFrame.width(); x += stepX)
        {
            diffSum += qAbs(backendSampleLuma(candidateLine[x]) - backendSampleLuma(referenceLine[x]));
            ++samples;
        }
    }

    if (samples <= 0)
    {
        return false;
    }

    return (diffSum / static_cast<double>(samples)) <= 18.0;
}

std::unique_ptr<LongCaptureBackend> createBestBackend(const QRect &captureRect,
                                                      SelectionOverlay *overlay,
                                                      QImage *firstFrame)
{
    DesktopGdiCaptureBackend gdiReference;

    auto captureBackendFrame = [captureRect, overlay](LongCaptureBackend &backend)
    {
        ScopedOverlayCaptureSuppression suppression(overlay);
        return LongCaptureBackend::normalizeCapturedImage(backend.capture(captureRect));
    };

    auto tryBackend = [captureRect, overlay, firstFrame, &gdiReference, &captureBackendFrame](std::unique_ptr<LongCaptureBackend> backend) -> std::unique_ptr<LongCaptureBackend>
    {
        Q_UNUSED(captureRect)
        Q_UNUSED(overlay)

        if (!backend || !backend->isAvailable())
        {
            return nullptr;
        }

        const QImage frame = captureBackendFrame(*backend);
        if (frame.isNull())
        {
            return nullptr;
        }

        if (backend->backendName() != QStringLiteral("GDI"))
        {
            const QImage referenceFrame = captureBackendFrame(gdiReference);
            if (!referenceFrame.isNull() && !desktopFramesRoughlyMatch(frame, referenceFrame))
            {
                return nullptr;
            }
        }

        if (firstFrame != nullptr)
        {
            *firstFrame = frame;
        }

        return backend;
    };

    if (auto backend = tryBackend(std::make_unique<WgcCaptureBackend>()))
    {
        return backend;
    }

    if (auto backend = tryBackend(std::make_unique<DxgiDuplicationBackend>()))
    {
        return backend;
    }

    auto backend = std::make_unique<DesktopGdiCaptureBackend>();
    if (firstFrame != nullptr)
    {
        *firstFrame = captureBackendFrame(*backend);
    }
    return backend;
}
}

LongCaptureSessionController::LongCaptureSessionController(QObject *parent)
    : QObject(parent)
    , m_scrollDispatcher(std::make_unique<ScrollDispatcher>())
    , m_motionDetector(std::make_unique<MotionDetector>())
    , m_stableCollector(std::make_unique<StableFrameCollector>())
    , m_overlapMatcher(std::make_unique<OverlapMatcher>())
    , m_stitchComposer(std::make_unique<StitchComposer>())
{
    m_observeTimer.setSingleShot(true);
    connect(&m_observeTimer, &QTimer::timeout, this, &LongCaptureSessionController::onObserveTimeout);
}

LongCaptureSessionController::~LongCaptureSessionController()
{
    cancel();
    releaseCaptureSuppression();
}

bool LongCaptureSessionController::start(const QRect &selectedRect, WId overlayWinId)
{
    cancel();

    QWidget *overlayWidget = QWidget::find(overlayWinId);
    m_overlay = qobject_cast<SelectionOverlay *>(overlayWidget);

    m_stitchComposer->reset();
    m_wheelAccumulator = 0;
    m_forceTargetRefresh = true;
    m_requestSerial = 0;
    m_pendingRequests.clear();
    resetObservationState();

    ShowTip::setCaptureSuppressed(true);

    QImage firstFrame;
    m_backend = createBestBackend(selectedRect, m_overlay, &firstFrame);
    if (m_backend == nullptr || firstFrame.isNull())
    {
        releaseCaptureSuppression();
        return false;
    }

    if (!m_stitchComposer->begin(firstFrame))
    {
        m_backend.reset();
        releaseCaptureSuppression();
        return false;
    }

    if (!m_session.begin(selectedRect, overlayWinId, firstFrame, m_backend->backendName()))
    {
        m_stitchComposer->reset();
        m_backend.reset();
        releaseCaptureSuppression();
        return false;
    }

    setCaptureQuality(CaptureQuality::Confirmed);
    updateOverlayState();

    emit previewUpdated(m_session.previewPixmap());
    emit committedVisualHeightChanged(m_session.committedVisualHeight());
    emit predictedVisualHeightChanged(m_session.predictedVisualHeight());
    emit statusTextChanged(QStringLiteral("长截图已开启（%1）").arg(m_session.backendName()));
    return true;
}

void LongCaptureSessionController::requestManualScroll(int delta)
{
    if (!isActive() || delta == 0)
    {
        return;
    }

    m_wheelAccumulator += delta;
    while (qAbs(m_wheelAccumulator) >= 120)
    {
        const int stepDelta = (m_wheelAccumulator > 0) ? 120 : -120;
        PendingRequest request;
        request.id = ++m_requestSerial;
        request.delta = stepDelta;
        request.predictedAppendedHeight = m_session.predictionStepHeight();
        m_session.setLastRequestId(request.id);
        m_session.adjustPredictedVisualHeight(request.predictedAppendedHeight);
        m_pendingRequests.enqueue(request);
        m_wheelAccumulator -= stepDelta;

        emit predictedVisualHeightChanged(m_session.predictedVisualHeight());
        setCaptureQuality(CaptureQuality::Predicting);
        emit statusTextChanged(QStringLiteral("预测追加 %1 px").arg(request.predictedAppendedHeight));
    }

    processPendingRequests();
}

void LongCaptureSessionController::confirmCopy()
{
    if (!isActive() || !m_stitchComposer->hasResult())
    {
        emit failed(QStringLiteral("长截图复制失败"));
        return;
    }

    m_session.setState(LongCaptureSession::State::Completed);
    m_observeTimer.stop();
    releaseCaptureSuppression();
    emit copyReady(m_stitchComposer->resultPixmap());
}

void LongCaptureSessionController::saveAs()
{
    if (!isActive() || !m_stitchComposer->hasResult())
    {
        emit failed(QStringLiteral("长截图保存失败"));
        return;
    }

    m_session.setState(LongCaptureSession::State::Completed);
    m_observeTimer.stop();
    releaseCaptureSuppression();
    emit saveReady(m_stitchComposer->resultPixmap());
}

void LongCaptureSessionController::cancel()
{
    m_observeTimer.stop();
    resetObservationState();
    m_wheelAccumulator = 0;
    m_forceTargetRefresh = true;
    m_pendingRequests.clear();
    m_scrollDispatcher->reset();
    m_stitchComposer->reset();
    m_backend.reset();

    if (m_overlay != nullptr)
    {
        m_overlay->setCaptureDecorationsHidden(false);
        m_overlay->setCaptureQuality(CaptureQuality::Idle);
        m_overlay->setLongCaptureModeEnabled(false);
        m_overlay->setCommittedLongCaptureHeight(0);
        m_overlay->setPredictedLongCaptureHeight(0);
        m_overlay->setLongCapturePreview(QPixmap());
        m_overlay->setStatusText(QString());
    }

    if (m_session.state() != LongCaptureSession::State::Idle)
    {
        m_session.setState(LongCaptureSession::State::Canceled);
    }

    m_session.reset();
    setCaptureQuality(CaptureQuality::Idle);
    releaseCaptureSuppression();
}

bool LongCaptureSessionController::isActive() const
{
    const LongCaptureSession::State state = m_session.state();
    return state != LongCaptureSession::State::Idle
           && state != LongCaptureSession::State::Completed
           && state != LongCaptureSession::State::Canceled;
}

LongCaptureSession::State LongCaptureSessionController::state() const
{
    return m_session.state();
}

void LongCaptureSessionController::onObserveTimeout()
{
    if (!isActive() || !m_activeRequest.valid)
    {
        return;
    }

    const CaptureFrame frame = captureObservationFrame();
    if (!frame.isValid())
    {
        handleObservationFailure(QStringLiteral("抓帧失败"));
        return;
    }

    if (!m_activeRequest.motionLocked)
    {
        ++m_activeRequest.motionAttempts;
        const MotionAnalysis analysis = m_motionDetector->analyze(m_stitchComposer->lastAcceptedFrame(), frame.image);
        const bool allowWeakMotion = (!analysis.moved
                                      && analysis.estimatedShiftPx > 0
                                      && analysis.confidence >= 0.42
                                      && m_activeRequest.motionAttempts >= 2);
        if (!analysis.moved && !allowWeakMotion)
        {
            if (m_activeRequest.motionAttempts >= 6)
            {
                m_forceTargetRefresh = true;
                handleObservationFailure(analysis.reason.isEmpty() ? QStringLiteral("未检测到滚动位移") : analysis.reason);
                return;
            }

            setCaptureQuality(CaptureQuality::WaitingForMotion);
            m_observeTimer.start(55);
            return;
        }

        m_activeRequest.motion = analysis;
        m_activeRequest.motionLocked = true;
        m_stableCollector->begin(frame);
        const QPixmap transientPreview = buildTransientPreview(frame.image,
                                                              analysis.estimatedShiftPx > 0
                                                                  ? analysis.estimatedShiftPx
                                                                  : m_activeRequest.predictedAppendedHeight);
        if (!transientPreview.isNull())
        {
            emit previewUpdated(transientPreview);
        }
        setCaptureQuality(CaptureQuality::WaitingForStable);
        emit statusTextChanged(allowWeakMotion ? QStringLiteral("位移弱确认，等待画面稳定") : QStringLiteral("等待画面稳定"));
        m_observeTimer.start(50);
        return;
    }

    ++m_activeRequest.stableAttempts;
    const StableFrameResult stableResult = m_stableCollector->ingest(frame);
    const QPixmap transientPreview = buildTransientPreview(frame.image,
                                                          m_activeRequest.motion.estimatedShiftPx > 0
                                                              ? m_activeRequest.motion.estimatedShiftPx
                                                              : m_activeRequest.predictedAppendedHeight);
    if (!transientPreview.isNull())
    {
        emit previewUpdated(transientPreview);
    }
    if (!stableResult.valid)
    {
        if (m_activeRequest.stableAttempts >= 12)
        {
            if (tryCommitFrame(frame.image, QStringLiteral("低稳定度")))
            {
                return;
            }

            handleObservationFailure(QStringLiteral("等待稳定帧超时"));
            return;
        }

        setCaptureQuality(CaptureQuality::WaitingForStable);
        m_observeTimer.start(50);
        return;
    }

    if (tryCommitFrame(stableResult.frame))
    {
        return;
    }

    if (m_activeRequest.stableAttempts >= 14)
    {
        handleObservationFailure(QStringLiteral("本次追加未通过校验"));
        return;
    }

    setCaptureQuality(CaptureQuality::WaitingForStable);
    m_observeTimer.start(45);
}

QPoint LongCaptureSessionController::scrollInjectionPoint() const
{
    const QRect captureRect = m_session.captureRect();
    const QPoint cursorPos = QCursor::pos();
    if (captureRect.contains(cursorPos))
    {
        return cursorPos;
    }

    return captureRect.center();
}

bool LongCaptureSessionController::ensureTargetResolved(bool forceRefresh)
{
    if (!forceRefresh && m_scrollDispatcher->hasTarget())
    {
        return true;
    }

    const bool shouldTemporarilyHideOverlay = (m_overlay != nullptr && m_overlay->isVisible());
    if (shouldTemporarilyHideOverlay)
    {
        m_overlay->hide();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 16);
        QThread::msleep(12);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 16);
    }

    const bool resolved = m_scrollDispatcher->resolveTarget(scrollInjectionPoint());

    if (shouldTemporarilyHideOverlay)
    {
        m_overlay->show();
        m_overlay->raise();
        m_overlay->activateWindow();
        m_overlay->setFocus();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 12);
    }

    return resolved;
}

bool LongCaptureSessionController::dispatchNextRequest()
{
    if (!isActive() || m_observeTimer.isActive() || m_activeRequest.valid || m_pendingRequests.isEmpty())
    {
        return false;
    }

    const PendingRequest request = m_pendingRequests.dequeue();
    m_activeRequest = ActiveRequest();
    m_activeRequest.valid = true;
    m_activeRequest.id = request.id;
    m_activeRequest.delta = request.delta;
    m_activeRequest.predictedAppendedHeight = request.predictedAppendedHeight;
    return dispatchActiveRequest();
}

bool LongCaptureSessionController::dispatchActiveRequest()
{
    if (!isActive() || !m_activeRequest.valid || m_observeTimer.isActive())
    {
        return false;
    }

    if (!ensureTargetResolved(m_forceTargetRefresh))
    {
        m_forceTargetRefresh = true;
        handleObservationFailure(QStringLiteral("未找到滚动目标"));
        return false;
    }

    m_forceTargetRefresh = false;

    ScrollDispatchResult dispatchResult;
    {
        ScopedOverlayInputPassthrough overlayPassthrough(m_overlay);
        dispatchResult = m_scrollDispatcher->dispatchWheel(m_activeRequest.delta, scrollInjectionPoint());
    }
    if (!dispatchResult.dispatched)
    {
        m_forceTargetRefresh = true;
        handleObservationFailure(QStringLiteral("滚动投递失败"));
        return false;
    }

    beginObservation();
    return true;
}

void LongCaptureSessionController::beginObservation()
{
    setOverlayCaptureSuppressed(true);
    if (m_backend != nullptr)
    {
        m_backend->beginObservation(m_session.captureRect());
    }

    m_activeRequest.motionAttempts = 0;
    m_activeRequest.stableAttempts = 0;
    m_activeRequest.motionLocked = false;
    m_activeRequest.motion = MotionAnalysis();
    m_stableCollector->reset();
    m_session.setState(LongCaptureSession::State::Observing);
    setCaptureQuality(CaptureQuality::WaitingForMotion);
    emit statusTextChanged(QStringLiteral("等待滚动位移"));
    m_observeTimer.start(70);
}

void LongCaptureSessionController::finishObservation()
{
    resetObservationState();
    m_session.setState(LongCaptureSession::State::Armed);
    processPendingRequests();
}

void LongCaptureSessionController::handleObservationFailure(const QString &reason,
                                                           MatchRejectReason rejectReason)
{
    if (!m_activeRequest.valid)
    {
        return;
    }

    if ((reason == QStringLiteral("未检测到滚动位移") || reason == QStringLiteral("画面变化过小"))
        && m_scrollDispatcher->advanceFallbackTarget())
    {
        emit statusTextChanged(QStringLiteral("切换滚动目标后重试"));
        resetObservationState(true);
        m_session.setState(LongCaptureSession::State::Armed);
        if (dispatchActiveRequest())
        {
            return;
        }
    }

    m_session.noteFailedRequest();
    m_session.adjustPredictedVisualHeight(-m_activeRequest.predictedAppendedHeight);
    emit previewUpdated(m_session.previewPixmap());
    emit predictedVisualHeightChanged(m_session.predictedVisualHeight());
    setCaptureQuality(CaptureQuality::WeakMatch);
    emit statusTextChanged(failureStatusText(reason, rejectReason));

    resetObservationState();
    m_session.setState(LongCaptureSession::State::Armed);
    processPendingRequests();
}

bool LongCaptureSessionController::tryCommitFrame(const QImage &frame, const QString &successTextSuffix)
{
    if (!m_activeRequest.valid || frame.isNull())
    {
        return false;
    }

    const int preferredShift = m_session.lastStableAppendHeight() > 0
                                   ? m_session.lastStableAppendHeight()
                                   : m_activeRequest.motion.estimatedShiftPx;
    const MatchDecision decision = m_overlapMatcher->match(m_stitchComposer->lastAcceptedFrame(),
                                                           frame,
                                                           m_activeRequest.motion.estimatedShiftPx,
                                                           preferredShift);

    int appendedHeight = decision.appendedHeight;
    QString commitMode;
    if (decision.accepted && m_stitchComposer->append(frame, decision.appendedHeight))
    {
        commitMode = successTextSuffix;
    }
    else
    {
        const int motionFallbackHeight = qBound(4,
                                                m_activeRequest.motion.estimatedShiftPx > 0
                                                    ? m_activeRequest.motion.estimatedShiftPx
                                                    : preferredShift,
                                                qMax(4, frame.height() - 1));
        if (motionFallbackHeight <= 0 || !m_stitchComposer->append(frame, motionFallbackHeight))
        {
            return false;
        }

        appendedHeight = motionFallbackHeight;
        commitMode = successTextSuffix.isEmpty()
                         ? QStringLiteral("位移回退")
                         : successTextSuffix + QStringLiteral(" · 位移回退");
    }

    const QPixmap preview = m_stitchComposer->resultPixmap();
    m_session.recordCommittedAppend(appendedHeight);
    m_session.updateResult(m_stitchComposer->lastAcceptedFrame(),
                           preview,
                           m_stitchComposer->visualHeight());
    m_session.resetFailedRequests();
    m_session.adjustPredictedVisualHeight(appendedHeight - m_activeRequest.predictedAppendedHeight);
    m_session.setState(LongCaptureSession::State::Stitching);

    emit previewUpdated(preview);
    emit committedVisualHeightChanged(m_session.committedVisualHeight());
    emit predictedVisualHeightChanged(m_session.predictedVisualHeight());
    setCaptureQuality(CaptureQuality::Confirmed);

    QString successText = QStringLiteral("已确认新增 %1 px（%2")
                              .arg(appendedHeight)
                              .arg(m_session.backendName());
    if (!commitMode.isEmpty())
    {
        successText += QStringLiteral(" · %1").arg(commitMode);
    }
    successText += QLatin1Char(')');
    emit statusTextChanged(successText);

    finishObservation();
    return true;
}

CaptureFrame LongCaptureSessionController::captureObservationFrame()
{
    if (m_backend != nullptr)
    {
        const CaptureFrame backendFrame = m_backend->tryGetLatestFrame();
        if (backendFrame.isValid())
        {
            return backendFrame;
        }
    }

    CaptureFrame fallbackFrame;
    fallbackFrame.image = captureFrame(m_session.captureRect());
    if (!fallbackFrame.image.isNull())
    {
        fallbackFrame.timestampMs = QDateTime::currentMSecsSinceEpoch();
        fallbackFrame.serial = static_cast<quint64>(m_activeRequest.id);
        if (m_backend != nullptr)
        {
            m_backend->beginObservation(m_session.captureRect());
        }
    }
    return fallbackFrame;
}

QPixmap LongCaptureSessionController::buildTransientPreview(const QImage &frame, int appendedHeight) const
{
    if (frame.isNull())
    {
        return QPixmap();
    }

    QImage base = m_session.previewPixmap().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (base.isNull())
    {
        return QPixmap();
    }

    QImage current = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (current.size() != m_session.lastAcceptedFrame().size())
    {
        current = current.scaled(m_session.lastAcceptedFrame().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const int safeAppendHeight = qBound(1,
                                        appendedHeight,
                                        qMax(1, current.height() - 1));
    const QImage appendStrip = current.copy(0,
                                            current.height() - safeAppendHeight,
                                            current.width(),
                                            safeAppendHeight);
    if (appendStrip.isNull())
    {
        return QPixmap();
    }

    QImage composed(base.width(),
                    base.height() + appendStrip.height(),
                    QImage::Format_ARGB32_Premultiplied);
    composed.fill(Qt::transparent);

    QPainter painter(&composed);
    painter.drawImage(0, 0, base);
    painter.drawImage(0, base.height(), appendStrip);

    painter.fillRect(QRect(0, base.height(), composed.width(), appendStrip.height()), QColor(104, 162, 235, 38));
    QPen pendingPen(QColor(168, 208, 255, 185), 1.0, Qt::DashLine);
    pendingPen.setDashPattern({5.0, 4.0});
    painter.setPen(pendingPen);
    painter.drawRect(QRect(0, base.height(), composed.width() - 1, appendStrip.height() - 1));
    painter.end();

    return QPixmap::fromImage(composed);
}

QImage LongCaptureSessionController::captureFrame()
{
    return captureFrame(m_session.captureRect());
}

QImage LongCaptureSessionController::captureFrame(const QRect &captureRect)
{
    if (m_backend == nullptr || captureRect.width() <= 1 || captureRect.height() <= 1)
    {
        return QImage();
    }

    if (m_overlayCaptureSuppressed)
    {
        return m_backend->capture(captureRect);
    }

    ScopedOverlayCaptureSuppression suppression(m_overlay);
    return m_backend->capture(captureRect);
}

void LongCaptureSessionController::setOverlayCaptureSuppressed(bool suppressed)
{
    if (m_overlayCaptureSuppressed == suppressed)
    {
        return;
    }

    m_overlayCaptureSuppressed = suppressed;
    if (m_overlay != nullptr && m_overlay->isVisible())
    {
        m_overlay->setCaptureDecorationsHidden(suppressed);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
    }
}

void LongCaptureSessionController::releaseCaptureSuppression()
{
    ShowTip::setCaptureSuppressed(false);
}

void LongCaptureSessionController::updateOverlayState()
{
    if (m_overlay == nullptr)
    {
        return;
    }

    m_overlay->setLongCaptureModeEnabled(isActive());
    m_overlay->setCommittedLongCaptureHeight(m_session.committedVisualHeight());
    m_overlay->setPredictedLongCaptureHeight(m_session.predictedVisualHeight());
    m_overlay->setLongCapturePreview(m_session.previewPixmap());
    m_overlay->setCaptureQuality(m_captureQuality);
}

void LongCaptureSessionController::resetObservationState(bool keepActiveRequest)
{
    m_observeTimer.stop();
    if (m_backend != nullptr)
    {
        m_backend->endObservation();
    }
    setOverlayCaptureSuppressed(false);
    m_stableCollector->reset();
    if (!keepActiveRequest)
    {
        m_activeRequest = ActiveRequest();
    }
}

void LongCaptureSessionController::processPendingRequests()
{
    if (!isActive() || m_observeTimer.isActive() || m_activeRequest.valid)
    {
        return;
    }

    dispatchNextRequest();
}

void LongCaptureSessionController::setCaptureQuality(CaptureQuality quality)
{
    if (m_captureQuality == quality)
    {
        return;
    }

    m_captureQuality = quality;
    emit captureQualityChanged(quality);
    if (m_overlay != nullptr)
    {
        m_overlay->setCaptureQuality(quality);
    }
}

QString LongCaptureSessionController::failureStatusText(const QString &reason,
                                                        MatchRejectReason rejectReason) const
{
    if (m_session.failedRequestCount() >= 2
        && (rejectReason == MatchRejectReason::SeamBreakReject
            || rejectReason == MatchRejectReason::WeakWinnerReject
            || rejectReason == MatchRejectReason::LowConfidenceReject))
    {
        return QStringLiteral("继续滚动到更稳定位置");
    }

    if (reason.isEmpty())
    {
        return QStringLiteral("本次追加未通过校验");
    }

    return reason;
}

















