#include "longcapturesessioncontroller.h"

#include "desktopgdicapturebackend.h"
#include "motiondetector.h"
#include "overlapmatcher.h"
#include "scrolldispatcher.h"
#include "selectionoverlay.h"
#include "showtip.h"
#include "stableframecollector.h"
#include "stitchcomposer.h"

#include <QApplication>
#include <QCursor>
#include <QEventLoop>
#include <QThread>
#include <QWidget>

LongCaptureSessionController::LongCaptureSessionController(QObject *parent)
    : QObject(parent)
    , m_backend(std::make_unique<DesktopGdiCaptureBackend>())
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
    releaseCaptureSuppression();
}

bool LongCaptureSessionController::start(const QRect &selectedRect, WId overlayWinId)
{
    cancel();

    QWidget *overlayWidget = QWidget::find(overlayWinId);
    m_overlay = qobject_cast<SelectionOverlay *>(overlayWidget);

    m_stitchComposer->reset();
    m_stableCollector->reset();
    m_scrollDispatcher->reset();
    m_wheelAccumulator = 0;
    m_pendingManualSteps = 0;
    m_forceTargetRefresh = true;
    resetObservationState();

    ShowTip::setCaptureSuppressed(true);

    const QImage firstFrame = captureFrame(selectedRect);
    if (firstFrame.isNull())
    {
        releaseCaptureSuppression();
        return false;
    }

    if (!m_stitchComposer->begin(firstFrame))
    {
        releaseCaptureSuppression();
        return false;
    }

    if (!m_session.begin(selectedRect, overlayWinId, firstFrame))
    {
        m_stitchComposer->reset();
        releaseCaptureSuppression();
        return false;
    }

    updateOverlayState();

    emit previewUpdated(m_session.previewPixmap());
    emit visualHeightChanged(m_session.visualHeight());
    emit statusTextChanged(QStringLiteral("长截图已开启，滚轮向下追加内容"));
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
        const int step = (m_wheelAccumulator > 0) ? 1 : -1;
        m_pendingManualSteps += step;
        m_wheelAccumulator -= step * 120;
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
    m_pendingManualSteps = 0;
    m_forceTargetRefresh = true;
    m_scrollDispatcher->reset();
    m_stitchComposer->reset();
    m_stableCollector->reset();

    if (m_overlay != nullptr)
    {
        m_overlay->setCaptureDecorationsHidden(false);
        m_overlay->setLongCaptureModeEnabled(false);
        m_overlay->setLongCaptureVisualHeight(0);
        m_overlay->setLongCapturePreview(QPixmap());
        m_overlay->setStatusText(QString());
    }

    if (m_session.state() != LongCaptureSession::State::Idle)
    {
        m_session.setState(LongCaptureSession::State::Canceled);
    }

    m_session.reset();
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
    if (!isActive())
    {
        return;
    }

    const QImage frame = captureFrame();
    if (frame.isNull())
    {
        handleObservationFailure(QStringLiteral("抓帧失败"), false);
        return;
    }

    if (!m_activeRequest.motionLocked)
    {
        ++m_activeRequest.motionAttempts;
        const MotionAnalysis analysis = m_motionDetector->analyze(m_stitchComposer->lastAcceptedFrame(), frame);
        if (!analysis.moved)
        {
            if (m_activeRequest.motionAttempts >= 6)
            {
                m_forceTargetRefresh = true;
                handleObservationFailure(QStringLiteral("未检测到滚动位移"), false);
                return;
            }

            m_observeTimer.start(80);
            return;
        }

        m_activeRequest.motion = analysis;
        m_activeRequest.motionLocked = true;
        m_stableCollector->begin(frame);
        m_observeTimer.start(70);
        return;
    }

    ++m_activeRequest.stableAttempts;
    const StableFrameResult stableResult = m_stableCollector->ingest(frame);
    if (!stableResult.valid)
    {
        if (m_activeRequest.stableAttempts >= 7)
        {
            handleObservationFailure(QStringLiteral("等待稳定帧超时"), false);
            return;
        }

        m_observeTimer.start(70);
        return;
    }

    const MatchDecision decision = m_overlapMatcher->match(m_stitchComposer->lastAcceptedFrame(),
                                                           stableResult.frame,
                                                           m_activeRequest.motion.estimatedShiftPx);
    if (!decision.accepted)
    {
        handleObservationFailure(decision.reason, false);
        return;
    }

    if (!m_stitchComposer->append(stableResult.frame, decision.appendedHeight))
    {
        handleObservationFailure(QStringLiteral("拼接失败"), false);
        return;
    }

    const QPixmap preview = m_stitchComposer->resultPixmap();
    m_session.updateResult(m_stitchComposer->lastAcceptedFrame(),
                           preview,
                           m_stitchComposer->visualHeight());
    m_session.setState(LongCaptureSession::State::Stitching);

    emit previewUpdated(preview);
    emit visualHeightChanged(m_stitchComposer->visualHeight());
    emit statusTextChanged(QStringLiteral("滚轮向下继续追加内容"));

    finishObservation(true);
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
    if (!isActive() || m_observeTimer.isActive())
    {
        return false;
    }

    int delta = 0;
    if (m_pendingManualSteps != 0)
    {
        delta = (m_pendingManualSteps > 0) ? 120 : -120;
        m_pendingManualSteps += (m_pendingManualSteps > 0) ? -1 : 1;
    }
    else
    {
        return false;
    }

    if (!ensureTargetResolved(m_forceTargetRefresh))
    {
        m_forceTargetRefresh = true;
        handleObservationFailure(QStringLiteral("未找到滚动目标"), false);
        return false;
    }

    m_forceTargetRefresh = false;

    const ScrollDispatchResult dispatchResult = m_scrollDispatcher->dispatchWheel(delta, scrollInjectionPoint());
    if (!dispatchResult.dispatched)
    {
        m_forceTargetRefresh = true;
        handleObservationFailure(QStringLiteral("滚动投递失败"), false);
        return false;
    }

    beginObservation(false, delta);
    return true;
}

void LongCaptureSessionController::beginObservation(bool fromAuto, int delta)
{
    resetObservationState();
    m_activeRequest.id = ++m_requestSerial;
    m_activeRequest.fromAuto = fromAuto;
    m_activeRequest.delta = delta;

    m_session.setState(LongCaptureSession::State::Observing);
    m_observeTimer.start(120);
}

void LongCaptureSessionController::finishObservation(bool success)
{
    Q_UNUSED(success)

    resetObservationState();

    m_session.setState(LongCaptureSession::State::Armed);

    processPendingRequests();
}

void LongCaptureSessionController::handleObservationFailure(const QString &reason, bool countAsAutoFailure)
{
    Q_UNUSED(countAsAutoFailure)


    if (reason == QStringLiteral("未检测到滚动位移") && m_scrollDispatcher->advanceFallbackTarget())
    {
        emit statusTextChanged(QStringLiteral("切换滚动目标后重试"));
        resetObservationState();
        m_session.setState(LongCaptureSession::State::Armed);
        dispatchNextRequest();
        return;
    }

    emit statusTextChanged(reason);

    resetObservationState();
    m_session.setState(LongCaptureSession::State::Armed);
    processPendingRequests();
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

    const bool suppressSelectionBorder = (m_overlay != nullptr && m_overlay->isVisible());
    if (suppressSelectionBorder)
    {
        m_overlay->setCaptureDecorationsHidden(true);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
    }

    const QImage image = m_backend->capture(captureRect);

    if (suppressSelectionBorder)
    {
        m_overlay->setCaptureDecorationsHidden(false);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 6);
    }

    return image;
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
    m_overlay->setLongCaptureVisualHeight(m_session.visualHeight());
    m_overlay->setLongCapturePreview(m_session.previewPixmap());
}

void LongCaptureSessionController::resetObservationState()
{
    m_activeRequest = ActiveRequest();
    m_stableCollector->reset();
}

void LongCaptureSessionController::processPendingRequests()
{
    if (!isActive() || m_observeTimer.isActive())
    {
        return;
    }

    if (m_pendingManualSteps != 0)
    {
        dispatchNextRequest();
    }
}

