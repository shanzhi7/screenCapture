#include "longcapturesession.h"

#include <QtMath>

LongCaptureSession::LongCaptureSession()
{
}

bool LongCaptureSession::begin(const QRect &captureRect,
                               WId overlayWinId,
                               const QImage &firstFrame,
                               const QString &backendName)
{
    Q_UNUSED(overlayWinId)

    reset();
    if (captureRect.width() <= 1 || captureRect.height() <= 1 || firstFrame.isNull())
    {
        return false;
    }

    m_captureRect = captureRect;
    m_lastAcceptedFrame = firstFrame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    m_previewPixmap = QPixmap::fromImage(m_lastAcceptedFrame);
    m_committedVisualHeight = m_lastAcceptedFrame.height();
    m_predictedVisualHeight = m_committedVisualHeight;
    m_backendName = backendName;
    m_state = State::Armed;
    return true;
}

void LongCaptureSession::reset()
{
    m_state = State::Idle;
    m_captureRect = QRect();
    m_lastAcceptedFrame = QImage();
    m_previewPixmap = QPixmap();
    m_committedVisualHeight = 0;
    m_predictedVisualHeight = 0;
    m_recentCommittedAppends.clear();
    m_failedRequestCount = 0;
    m_lastRequestId = 0;
    m_backendName.clear();
}

LongCaptureSession::State LongCaptureSession::state() const
{
    return m_state;
}

void LongCaptureSession::setState(State state)
{
    m_state = state;
}

QRect LongCaptureSession::captureRect() const
{
    return m_captureRect;
}

QPixmap LongCaptureSession::previewPixmap() const
{
    return m_previewPixmap;
}

int LongCaptureSession::committedVisualHeight() const
{
    return m_committedVisualHeight;
}

int LongCaptureSession::predictedVisualHeight() const
{
    return m_predictedVisualHeight;
}

const QImage &LongCaptureSession::lastAcceptedFrame() const
{
    return m_lastAcceptedFrame;
}

void LongCaptureSession::updateResult(const QImage &lastAcceptedFrame,
                                      const QPixmap &preview,
                                      int committedVisualHeight)
{
    m_lastAcceptedFrame = lastAcceptedFrame;
    m_previewPixmap = preview;
    m_committedVisualHeight = qMax(0, committedVisualHeight);
    if (m_predictedVisualHeight < m_committedVisualHeight)
    {
        m_predictedVisualHeight = m_committedVisualHeight;
    }
}

void LongCaptureSession::setPredictedVisualHeight(int height)
{
    m_predictedVisualHeight = qMax(m_committedVisualHeight, height);
}

void LongCaptureSession::adjustPredictedVisualHeight(int delta)
{
    setPredictedVisualHeight(m_predictedVisualHeight + delta);
}

void LongCaptureSession::alignPredictedToCommitted()
{
    m_predictedVisualHeight = m_committedVisualHeight;
}

void LongCaptureSession::recordCommittedAppend(int appendedHeight)
{
    if (appendedHeight <= 0)
    {
        return;
    }

    m_recentCommittedAppends.append(appendedHeight);
    while (m_recentCommittedAppends.size() > 5)
    {
        m_recentCommittedAppends.removeFirst();
    }
}

int LongCaptureSession::predictionStepHeight() const
{
    if (m_recentCommittedAppends.isEmpty())
    {
        return qBound(48, m_captureRect.height() / 5, 160);
    }

    QList<int> sorted = m_recentCommittedAppends;
    std::sort(sorted.begin(), sorted.end());
    if (sorted.size() > 2)
    {
        sorted.removeFirst();
        sorted.removeLast();
    }

    int sum = 0;
    for (int value : sorted)
    {
        sum += value;
    }

    return qBound(32, sum / qMax(1, sorted.size()), 420);
}

int LongCaptureSession::lastStableAppendHeight() const
{
    return m_recentCommittedAppends.isEmpty() ? 0 : m_recentCommittedAppends.constLast();
}

void LongCaptureSession::noteFailedRequest()
{
    ++m_failedRequestCount;
}

void LongCaptureSession::resetFailedRequests()
{
    m_failedRequestCount = 0;
}

int LongCaptureSession::failedRequestCount() const
{
    return m_failedRequestCount;
}

void LongCaptureSession::setLastRequestId(int requestId)
{
    m_lastRequestId = requestId;
}

int LongCaptureSession::lastRequestId() const
{
    return m_lastRequestId;
}

QString LongCaptureSession::backendName() const
{
    return m_backendName;
}

