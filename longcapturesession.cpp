#include "longcapturesession.h"

LongCaptureSession::LongCaptureSession()
{
}

bool LongCaptureSession::begin(const QRect &captureRect, WId overlayWinId, const QImage &firstFrame)
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
    m_visualHeight = m_lastAcceptedFrame.height();
    m_state = State::Armed;
    return true;
}

void LongCaptureSession::reset()
{
    m_state = State::Idle;
    m_captureRect = QRect();
    m_lastAcceptedFrame = QImage();
    m_previewPixmap = QPixmap();
    m_visualHeight = 0;
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

int LongCaptureSession::visualHeight() const
{
    return m_visualHeight;
}

const QImage &LongCaptureSession::lastAcceptedFrame() const
{
    return m_lastAcceptedFrame;
}

void LongCaptureSession::updateResult(const QImage &lastAcceptedFrame, const QPixmap &preview, int visualHeight)
{
    m_lastAcceptedFrame = lastAcceptedFrame;
    m_previewPixmap = preview;
    m_visualHeight = visualHeight;
}
