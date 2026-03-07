/***********************************************************************************
*
* @file         captureuistatecoordinator.h
* @brief        截图 UI 状态协调器：管理托盘隐藏与截图会话窗口恢复策略。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef CAPTUREUISTATECOORDINATOR_H
#define CAPTUREUISTATECOORDINATOR_H

class CaptureUiStateCoordinator
{
public:
    CaptureUiStateCoordinator() = default;

    // 标记主窗口是否处于“隐藏到托盘”状态。
    void markHiddenToTray(bool hidden);

    // 查询当前托盘隐藏状态。
    bool isHiddenToTray() const;

    // 开始一次截图会话，并固化本次会话的窗口恢复策略。
    void beginCaptureSession(bool hiddenToTray);

    // 结束截图会话，恢复默认窗口恢复策略。
    void endCaptureSession();

    // 当前会话完成后是否应恢复主窗口。
    bool shouldRestoreMainWindowAfterCapture() const;

private:
    bool m_hiddenToTray = false;
    bool m_captureSessionActive = false;
    bool m_captureKeepHidden = false;
};

#endif // CAPTUREUISTATECOORDINATOR_H
