#include "captureuistatecoordinator.h"

void CaptureUiStateCoordinator::markHiddenToTray(bool hidden)
{
    m_hiddenToTray = hidden;
}

bool CaptureUiStateCoordinator::isHiddenToTray() const
{
    return m_hiddenToTray;
}

void CaptureUiStateCoordinator::beginCaptureSession(bool hiddenToTray)
{
    m_captureSessionActive = true;
    m_captureKeepHidden = hiddenToTray;
}

void CaptureUiStateCoordinator::endCaptureSession()
{
    m_captureSessionActive = false;
    m_captureKeepHidden = false;
}

bool CaptureUiStateCoordinator::shouldRestoreMainWindowAfterCapture() const
{
    if (!m_captureSessionActive)
    {
        return true;
    }

    return !m_captureKeepHidden;
}
