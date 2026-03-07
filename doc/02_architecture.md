# 架构文档（同步版）

> 更新时间：2026-03-07

## 1. 架构分层

当前系统按职责分为七层：

1. UI 层：`MainWindow`、`.ui`、QSS
2. 交互层：`SelectionOverlay`
3. 拼接层：`LongCaptureStitcher`
4. 提示层：`ShowTip`（View） + `TipPresenter`（定位与展示协调）
5. 状态与决策层：`CaptureUiStateCoordinator`、`CaptureResultHandler`
6. 配置层：`SettingsService`
7. 平台集成层：系统托盘、全局热键、Windows 资源图标

## 2. 模块职责

### 2.1 MainWindow

- 主界面入口与模式调度
- 全屏截图（含多屏选择）
- 区域截图与长截图流程编排
- 依赖组装与事件分发（不直接承担提示定位、配置迁移和恢复决策细节）
- 关闭事件策略分发（调用 `ShowTip::askCloseChoice`）

### 2.2 SelectionOverlay

- 全屏遮罩与选区交互
- 工具条展示与事件发射
- 长截图状态下滚轮事件上报
- 选区可视高度随长截图增长

### 2.3 LongCaptureStitcher

- 管理长截图拼接状态
- 帧变化判定与滚动位移估算
- 增量拼接与结果输出

### 2.4 ShowTip + TipPresenter

- `ShowTip`：纯提示视图，负责文本展示与隐藏计时
- `TipPresenter`：提示定位策略（鼠标优先，越界回退屏幕右下角）与实例生命周期管理

### 2.5 CaptureUiStateCoordinator

- 管理主窗口是否隐藏到托盘
- 管理截图会话“是否恢复窗口”的状态

### 2.6 CaptureResultHandler

- 依据截图动作类型（复制/保存）与当前状态输出处理决策
- 给出“是否恢复窗口 / 是否走手动保存对话框”的统一判断

### 2.7 SettingsService

- 统一 `QSettings` 读写入口，避免业务层硬编码命名空间
- 首次启动自动迁移 `shanzhi/screenCapture` 到 `QingYing/LightShadowCapture`
- 迁移策略：新命名空间已有值优先，不覆盖

### 2.8 CloseDialog

- 关闭应用确认弹窗（隐藏到托盘 / 退出 / 取消）
- 通过 `.ui` + QSS 维持主界面同风格视觉

## 3. 关键数据流

- 全屏截图：`MainWindow::startCapture -> CaptureResultHandler -> (restoreWindowIfNeeded?) -> Clipboard/AutoSave`
- 区域截图：`MainWindow -> SelectionOverlay -> MainWindow::captureRegion -> CaptureResultHandler`
- 长截图：`SelectionOverlay(滚轮) -> MainWindow -> LongCaptureStitcher::append`
- 提示显示：`MainWindow::showTip -> TipPresenter::show -> ShowTip::showAt`
- 配置访问：`MainWindow -> SettingsService -> QSettings`
- 配置迁移：`main -> SettingsService::initializeAndMigrateLegacySettings -> Legacy/New QSettings`
- 关闭动作：`MainWindow::closeEvent -> ShowTip::askCloseChoice -> CloseDialog`

## 4. 当前架构结论

当前架构已完成一次中等解耦，并完成配置层抽离。后续建议继续拆分：

- `ScreenCaptureService`：抓屏与多屏坐标能力
- `HistoryService`：截图历史管理与缓存策略
