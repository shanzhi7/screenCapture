#include "captureresulthandler.h"

CaptureResultDecision CaptureResultHandler::decide(CaptureResultAction action,
                                                   bool shouldRestoreWindowByState,
                                                   bool autoSaveEnabled) const
{
    CaptureResultDecision decision;
    decision.shouldRestoreWindow = shouldRestoreWindowByState;

    if (action == CaptureResultAction::RegionSaveRequested)
    {
        decision.useManualSaveDialog = !autoSaveEnabled;
    }
    else if (action == CaptureResultAction::LongCaptureSaveRequested)
    {
        // 长截图保持手动保存。
        decision.useManualSaveDialog = true;
    }

    return decision;
}
