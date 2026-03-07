# 模块设计（同步版）

> 更新时间：2026-03-07

## 1. MainWindow

### 已实现接口（核心）

- `startCapture()`
- `onModeFullClicked()` / `onModeRegionClicked()`
- `onSelectionFinished(const QRect &)`
- `onOverlaySaveRequested(const QRect &)`
- `onOverlayLongCaptureToggled(bool, const QRect &)`
- `onOverlayLongCaptureWheel(const QRect &, int)`
- `onOverlayLongCaptureSaveRequested(const QRect &)`
- `onOverlayLongCaptureConfirmRequested(const QRect &)`
- `captureRegion(const QRect &) const`
- `restoreWindowIfNeeded()` / `endCaptureSession()`
- `closeEvent(QCloseEvent *)`
- `ensureTrayIcon()` / `onTrayExitRequested()` / `onTrayShowRequested()`

### 说明

- 负责流程编排与事件分发。
- 提示定位、窗口恢复决策、配置迁移等策略由独立类承担。

## 2. SelectionOverlay

### 已实现能力

- 全屏遮罩、拖拽选区
- 尺寸提示
- ESC 取消 / Enter 确认
- 工具条（编辑入口 + 长截图 + 保存/取消/确认）
- 长截图期间选区可视高度增长

### 关键约束

- 选区蓝色描边仅作为交互态视觉反馈，最终截图不带描边。

## 3. LongCaptureStitcher

### 已实现能力

- `begin(firstFrame)` 初始化
- `append(nextFrame)` 增量拼接并返回新增高度
- `resultPixmap()` 获取当前拼接结果
- `visualHeight()` 提供 UI 可视高度同步

### 设计价值

- 降低拼接算法与 UI 编排耦合
- 降低重复拼接和错位概率
- 便于独立测试和算法迭代

## 4. ShowTip

- `measureSize(...)`：计算提示尺寸
- `showAt(...)`：按给定全局坐标展示
- `askCloseChoice(...)`：统一触发关闭行为弹窗

## 5. TipPresenter

- `show(...)`：统一提示入口
- 默认策略：鼠标优先显示，越界回退当前屏幕右下角
- 管理 `ShowTip` 顶层实例，避免与主窗口隐藏状态耦合

## 6. CaptureUiStateCoordinator

- `markHiddenToTray(...)`
- `beginCaptureSession(...)`
- `shouldRestoreMainWindowAfterCapture()`
- `endCaptureSession()`

## 7. CaptureResultHandler

- `decide(...)`：输出截图动作决策（窗口恢复 / 手动保存）
- 统一处理“区域保存”和“长截图保存”的对话框策略

## 8. SettingsService

- `initializeAndMigrateLegacySettings()`：首次迁移旧配置
- `readBool/writeBool`：布尔配置统一读写
- `readString/writeString`：字符串配置统一读写
- 迁移标记：`meta/settings_migrated_from_legacy`

## 9. CloseDialog

- 以独立类承载退出流程交互
- 通过 `Choice` 枚举回传结果，避免业务散落
- 使用 `.ui` 文件维护布局，便于后续样式演进

## 10. 后续拆分建议

- `ScreenCaptureService`：屏幕抓取能力
- `HistoryService`：历史记录管理
