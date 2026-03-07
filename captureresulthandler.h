/***********************************************************************************
*
* @file         captureresulthandler.h
* @brief        截图结果处理决策器：根据动作与状态给出窗口恢复与保存策略。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef CAPTURERESULTHANDLER_H
#define CAPTURERESULTHANDLER_H

// 截图结果动作类型。
enum class CaptureResultAction
{
    FullScreenCaptured,
    RegionConfirmed,
    RegionSaveRequested,
    LongCaptureCopied,
    LongCaptureSaveRequested,
    SelectionCanceled
};

// 截图结果处理决策。
struct CaptureResultDecision
{
    bool shouldRestoreWindow = true;
    bool useManualSaveDialog = false;
};

class CaptureResultHandler
{
public:
    CaptureResultHandler() = default;

    // 根据当前动作生成处理决策。
    CaptureResultDecision decide(CaptureResultAction action,
                                 bool shouldRestoreWindowByState,
                                 bool autoSaveEnabled) const;
};

#endif // CAPTURERESULTHANDLER_H
