# 模块设计（同步版）

> 更新时间：2026-03-06

## 1. MainWindow

### 已实现接口（核心）

- `startCapture()`
- `onModeFullClicked()`
- `onModeRegionClicked()`
- `onSelectionFinished(const QRect &)`
- `onOverlaySaveRequested(const QRect &)`
- `onOverlayLongCaptureToggled(bool, const QRect &)`
- `onOverlayLongCaptureWheel(const QRect &, int)`
- `onOverlayLongCaptureSaveRequested(const QRect &)`
- `onOverlayLongCaptureConfirmRequested(const QRect &)`
- `captureRegion(const QRect &) const`

### 说明

- 负责流程编排，不再承担复杂拼接算法细节。

## 2. SelectionOverlay

### 已实现能力

- 全屏遮罩、拖拽选区
- 尺寸提示
- ESC 取消 / Enter 确认
- 工具条（编辑入口 + 长截图 + 保存/取消/确认）
- 长截图期间选区可视高度增长

## 3. LongCaptureStitcher（新增）

### 已实现能力

- `begin(firstFrame)` 初始化
- `append(nextFrame)` 增量拼接并返回新增高度
- `resultPixmap()` 获取当前拼接结果
- `visualHeight()` 提供 UI 可视高度同步

### 设计价值

- 降低 `MainWindow` 复杂度
- 降低重复拼接和错位概率
- 便于独立测试和迭代算法

## 4. ShowTip

- 统一提示入口：`showText(...)`
- 替代零散提示逻辑

## 5. 待拆分建议

- `ScreenCaptureService`：屏幕抓取能力
- `SettingsService`：配置持久化
- `HistoryService`：历史记录管理
