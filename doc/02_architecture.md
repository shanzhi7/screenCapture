# 架构文档（同步版）

> 更新时间：2026-03-06

## 1. 架构分层

当前系统按职责分为四层：

1. UI 层：`MainWindow`、`.ui`、QSS
2. 交互层：`SelectionOverlay`
3. 拼接层：`LongCaptureStitcher`
4. 反馈层：`ShowTip`

## 2. 模块职责

### 2.1 MainWindow

- 主界面入口与模式调度
- 全屏截图（含多屏选择）
- 区域截图流程管理
- 长截图流程编排（启停、保存、复制）
- 占位按钮统一处理

### 2.2 SelectionOverlay

- 全屏遮罩和选区交互
- 工具条展示与事件发射
- 长截图状态下滚轮事件上报
- 选区可视化高度随长截图增长

### 2.3 LongCaptureStitcher（新增）

- 管理长截图拼接状态
- 帧变化判定
- 位移估算（scroll shift）
- 增量拼接与结果输出

### 2.4 ShowTip

- 统一短时提示反馈

## 3. 关键数据流

- 区域截图：`MainWindow -> SelectionOverlay -> MainWindow::captureRegion`
- 长截图：`SelectionOverlay(滚轮) -> MainWindow -> LongCaptureStitcher::append`
- 输出：`LongCaptureStitcher::resultPixmap -> 预览/保存/复制`

## 4. 当前架构结论

相比旧版本，长截图已从“临时逻辑”升级为独立拼接模块，稳定性和可维护性显著提升；后续可继续将抓图能力抽到 `ScreenCaptureService`。
