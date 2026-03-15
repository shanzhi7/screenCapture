# 架构文档（同步版）

> 更新时间：2026-03-14

## 1. 当前架构分层

当前系统按职责分为七层：

1. UI 层：`MainWindow`、`.ui`、QSS
2. 交互层：`SelectionOverlay`
3. 历史与数据层：`CaptureHistoryManager`
4. 提示层：`ShowTip`（View） + `TipPresenter`（定位与展示协调）
5. 状态与决策层：`CaptureUiStateCoordinator`、`CaptureResultHandler`
6. 配置层：`SettingsService`
7. 实验长截图层（当前默认构建开启，仍按实验特性管理）：`LongCaptureSessionController` 与流水线模块

> 说明：长截图目前采用“实验特性 + 独立链路”的接入方式。当前仓库默认构建会编译该层，但标准截图主流程仍不依赖它。

## 2. 模块职责

### 2.1 MainWindow

- 主界面入口与模式调度
- 全屏截图（含多屏选择）
- 区域截图流程编排
- 截图结果预览、历史写入、剪贴板复制、保存对话框
- 托盘、热键、设置入口的依赖组装与事件分发
- 在实验开关开启时，负责把 `SelectionOverlay` 的长截图事件转发给 `LongCaptureSessionController`

### 2.2 SelectionOverlay

- 全屏遮罩与拖拽选区
- 选区尺寸提示
- 右键 / ESC 取消，Enter 确认
- 浮动工具条（画笔 / 矩形 / 圆形 / 马赛克 / 文字 / 撤销 / 重做 / 长截图 / 保存 / 取消 / 确认）
- 标注绘制、结果合成与样式子工具条
- 多屏环境下工具条位置约束
- 在实验开关开启时，额外挂接：
  - 长截图按钮
  - 右侧实时预览面板
  - 长截图滚轮转发

### 2.3 CaptureHistoryManager

- 截图历史持久化
- 最近截图列表读取
- 缩略图回显到右侧预览区

### 2.4 ShowTip + TipPresenter

- `ShowTip`：纯提示视图
- `TipPresenter`：负责提示定位策略与实例生命周期
- 当前策略：鼠标优先，越界回退屏幕右下角
- 长截图实验会话期间可通过 `ShowTip::setCaptureSuppressed(true)` 暂停顶层提示入镜

### 2.5 CaptureUiStateCoordinator

- 管理主窗口是否隐藏到托盘
- 管理截图会话是否需要恢复主窗口

### 2.6 CaptureResultHandler

- 根据截图动作类型输出流程决策
- 统一控制“是否恢复窗口 / 是否手动保存”

### 2.7 SettingsService

- 统一 `QSettings` 读写入口
- 自动迁移旧命名空间 `shanzhi/screenCapture`
- 当前命名空间：`QingYing/LightShadowCapture`

### 2.8 CloseDialog

- 关闭应用确认弹窗
- 提供“隐藏到托盘 / 退出 / 取消”三种选择

### 2.9 实验长截图层

#### LongCaptureSessionController

- 长截图会话外观控制器
- 负责 Overlay 事件接入、状态切换、结果回传
- 仅在首次进入或目标失效重解析时短暂隐藏 Overlay 一次
- 统一编排手动滚轮请求与长截图结果回传

#### LongCaptureSession

- 长截图状态容器
- 保存当前捕获区域、会话状态、最近接受帧、预览图与可视高度

#### LongCaptureBackend / DesktopGdiCaptureBackend

- `LongCaptureBackend`：抓帧抽象接口
- `DesktopGdiCaptureBackend`：Windows GDI 桌面抓帧实现
- 后续可在不改会话层的前提下替换为 WGC / DXGI 后端

#### ScrollDispatcher

- 解析滚动目标窗口
- 投递鼠标滚轮消息
- 当前采用 Windows `SendMessageTimeoutW(WM_MOUSEWHEEL)` 优先，`PostMessageW` 兜底

#### MotionDetector

- 判断滚动后画面是否真的发生位移
- 输出 `MotionAnalysis { moved, estimatedShiftPx, confidence }`

#### StableFrameCollector

- 在检测到位移后收集连续稳定帧
- 仅对稳定帧开放后续 overlap 匹配

#### OverlapMatcher

- 在位移约束窗口内搜索可靠 overlap
- 使用中心 ROI、右侧 ROI、接缝带三路校验
- 输出 `MatchDecision { accepted, appendedHeight, confidence, reason }`

#### StitchComposer

- 管理首帧初始化、条带追加、最终结果图与预览图生成
- 当前只追加“可靠新增条带”，不允许未位移兜底拼接

#### LongCapturePreviewPanel

- Overlay 右侧实时预览面板
- 展示当前拼接结果缩放视图与实际尺寸

## 3. 关键数据流

- 全屏截图：`MainWindow::startCapture -> 选择目标屏幕 -> Clipboard/AutoSave/Preview`
- 区域截图：`MainWindow -> SelectionOverlay -> MainWindow::captureRegion -> Clipboard/Save/Preview`
- 提示显示：`MainWindow::showTip -> TipPresenter::show -> ShowTip::showAt`
- 配置访问：`MainWindow -> SettingsService -> QSettings`
- 关闭动作：`MainWindow::closeEvent -> ShowTip::askCloseChoice -> CloseDialog`
- 实验长截图：
  - `SelectionOverlay -> LongCaptureSessionController`
  - `ScrollDispatcher -> MotionDetector -> StableFrameCollector -> OverlapMatcher -> StitchComposer`
  - `LongCaptureSessionController -> MainWindow / SelectionOverlay`

## 4. 当前架构结论

当前稳定主线仍聚焦在：

- 全屏截图
- 区域截图
- 保存 / 复制 / 自动保存
- 历史记录
- 托盘与全局热键
- 设置与提示系统

长截图已改为“实验特性 + 独立链路”的接入方式，避免继续污染稳定主线。当前仓库默认构建会包含该链路，但 T401/T402 完成的仍是 Windows GDI 手动滚轮版实验实现，不代表功能已验收通过。
