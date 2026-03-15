# 模块设计（同步版）

> 更新时间：2026-03-14

## 1. MainWindow

### 已实现接口（核心）

- `startCapture()`
- `onModeFullClicked()` / `onModeRegionClicked()`
- `onSelectionFinished(const QRect &)`
- `onSelectionCanceled()`
- `onOverlaySaveRequested(const QRect &)`
- `captureRegion(const QRect &) const`
- `restoreWindowIfNeeded()` / `endCaptureSession()`
- `closeEvent(QCloseEvent *)`
- `ensureTrayIcon()` / `onTrayExitRequested()` / `onTrayShowRequested()`

### 实验长截图挂接接口（仅在 `SCREENCAPTURE_ENABLE_LONG_CAPTURE=1` 时编译）

- `onOverlayLongCaptureToggled(bool, const QRect &)`
- `onOverlayLongCaptureWheel(const QRect &, int)`
- `onOverlayLongCaptureSaveRequested(const QRect &)`
- `onOverlayLongCaptureConfirmRequested(const QRect &)`
- `onLongCapturePreviewUpdated(const QPixmap &)`
- `onLongCapturePredictedVisualHeightChanged(int)`
- `onLongCaptureCommittedVisualHeightChanged(int)`
- `onLongCaptureCaptureQualityChanged(CaptureQuality)`
- `onLongCaptureStatusTextChanged(const QString &)`
- `onLongCaptureCopyReady(const QPixmap &)`
- `onLongCaptureSaveReady(const QPixmap &)`
- `onLongCaptureFailed(const QString &)`

### 说明

- `MainWindow` 负责截图主流程编排与事件分发。
- T401 中不把长截图算法堆回 `MainWindow`，只保留依赖组装与结果转发。

## 2. SelectionOverlay

### 已实现能力

- 全屏遮罩、拖拽选区
- 尺寸提示
- ESC / 右键取消，Enter 确认
- 浮动工具条（画笔 / 矩形 / 圆形 / 马赛克 / 文字 / 撤销 / 重做 / 长截图 / 保存 / 取消 / 确认）
- 标注绘制结果合成与样式子工具条

### 实验长截图能力（按编译开关接入，当前默认构建开启）

- `setLongCaptureModeEnabled(bool)`
- `setPredictedLongCaptureHeight(int)`
- `setCommittedLongCaptureHeight(int)`
- `setLongCapturePreview(const QPixmap &)`
- `setStatusText(const QString &)`
- `setCaptureQuality(CaptureQuality)`
- `setCaptureDecorationsHidden(bool)`
- `longCaptureToggled(bool, const QRect &)`
- `longCaptureWheel(const QRect &, int)`
- `longCaptureSaveRequested(const QRect &)`
- `longCaptureConfirmRequested(const QRect &)`

### 关键约束

- 选区蓝色描边仅作为交互态视觉反馈，最终截图不带描边。
- 主工具条与子工具条会按整块区域一起避让当前屏幕边界，避免贴边选区时跑出屏幕。
- 长截图抓帧期间会统一隐藏主工具条、子工具条、状态提示与预览面板，避免 Overlay 装饰入图。
- 长截图预览面板与按钮只在实验开关开启时参与构建。

## 3. CaptureHistoryManager

### 已实现能力

- 截图历史写入与读取
- 最近截图列表数据模型
- 缩略图点击回显预览

## 4. CaptureUiStateCoordinator / CaptureResultHandler

### 已实现能力

- 托盘隐藏状态管理
- 截图会话恢复窗口决策
- 保存动作与复制动作统一决策

## 5. SettingsService

### 已实现能力

- 配置统一读写
- 旧配置自动迁移
- 输出格式、自动保存、热键、目录等配置持久化

## 6. GlobalHotkeyManager

### 已实现能力

- Windows 全局热键注册
- 热键变更与失败回滚
- 托盘隐藏状态下依然可触发截图

## 7. ShowTip / TipPresenter / CloseDialog

- `ShowTip`：纯展示组件
- `TipPresenter`：提示定位与生命周期管理
- `CloseDialog`：关闭应用选择弹窗

## 8. T401 实验长截图模块

### 8.1 LongCaptureSessionController

#### 对外接口

- `bool start(const QRect &selectedRect, WId overlayWinId)`
- `void requestManualScroll(int delta)`
- `void confirmCopy()`
- `void saveAs()`
- `void cancel()`

#### 职责

- 管理长截图会话生命周期
- 仅在首次进入或目标失效重解析时短暂隐藏 Overlay 一次
- 串联滚动投递、位移检测、稳定帧采样、overlap 匹配、结果拼接
- 通过信号把预览、状态文案、复制/保存结果回传给上层

### 8.2 LongCaptureSession

#### 已定义状态

- `Idle`
- `Armed`
- `Observing`
- `Stitching`
- `Completed`
- `Canceled`

#### 职责

- 保存捕获区域、Overlay `WId`、最近接受帧、预览图与可视高度

### 8.3 LongCaptureBackend / DesktopGdiCaptureBackend

- `LongCaptureBackend::capture(const QRect &globalRect)`：抓取全局区域图像
- `DesktopGdiCaptureBackend`：当前 Windows GDI 实现
- 后续 T403 将基于该接口接入 WGC / DXGI

### 8.4 ScrollDispatcher

- `resolveTarget(const QPoint &screenPos)`：锁定滚动目标
- `dispatchWheel(int delta, const QPoint &screenPos)`：投递滚轮消息
- 当前采用 `SendMessageTimeoutW` 优先，`PostMessageW` 兜底
- `hasTarget()` / `reset()`：维护目标窗口状态

### 8.5 MotionDetector

- `analyze(const QImage &previousFrame, const QImage &currentFrame)`
- 输出 `MotionAnalysis`，用于判断是否真的发生滚动以及估算 `estimatedShiftPx`

### 8.6 StableFrameCollector

- `begin(const QImage &firstFrame)`
- `ingest(const QImage &frame)`
- `reset()`
- 只在检测到位移后收集稳定帧，避免把滚动中的帧直接送去匹配

### 8.7 OverlapMatcher

- `match(const QImage &previousFrame, const QImage &currentFrame, int expectedShiftPx)`
- 采用位移约束窗口、中心 ROI、右侧 ROI、接缝带三路校验
- 输出 `MatchDecision`

### 8.8 StitchComposer

- `begin(const QImage &firstFrame)`
- `append(const QImage &frame, int appendedHeight)`
- `resultPixmap()` / `lastAcceptedFrame()` / `visualHeight()`
- 当前仅负责可靠新增条带的顺接拼接

### 8.9 LongCapturePreviewPanel

- Overlay 右侧预览视图组件
- 当前展示拼接结果缩放图与尺寸信息

## 9. 当前状态结论

- 标准全屏截图与普通区域截图仍是稳定主线。
- T401 已把长截图以实验特性形式重新接回；当前仓库默认构建会包含该链路，但仍不视为已验收功能。
- 后续若要开放入口，前提是先完成 Windows 场景下的实测验收，再立项 T403 升级抓帧后端。
