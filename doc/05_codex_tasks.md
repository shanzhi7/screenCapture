# Codex 任务清单（同步版）

> 更新时间：2026-03-09

## 已完成

### T001 主界面重构

- 主界面布局与 QSS 主题完成

### T002 基础截图闭环

- 全屏截图、区域截图、保存、复制完成

### T003 覆盖层交互

- 选区交互、工具条、确认/取消完成

### T004 提示系统

- `ShowTip` 已接入全局提示链路

### T101 长截图第一阶段（已回退）

- 旧版长截图链路已从主流程回退
- 当前不再沿用旧实现继续叠加修补

### T102 全屏多屏选择

- 全屏截图支持手动选择目标显示器

### T103 截图输出一致性修正

- 选区交互态保留描边，最终截图结果移除描边
- 修复 Overlay 关闭与状态残留问题

### T104 全局热键与托盘能力

- Windows 全局热键 `Ctrl+Shift+A`
- 系统托盘显示/退出能力接入

### T105 关闭弹窗重构

- 新增 `CloseDialog`（`.h/.cpp/.ui`）
- `ShowTip::askCloseChoice` 改为调用 `CloseDialog`

### T106 应用图标接入

- `icon.ico` 作为窗口图标
- `appicon.rc` 接入 EXE 图标

### T202 历史记录面板与数据持久化（第一阶段）

- 新增 `CaptureHistoryManager`，自动落盘截图历史
- 最近截图区域改为读取真实历史数据
- 支持点击历史缩略图回显到预览区

### T203 输出格式配置（PNG/JPG）

- 新增 PNG/JPG 输出格式切换
- 保存对话框默认扩展名与当前格式联动
- 输出格式通过 `QSettings` 持久化

### T204 自动保存真实逻辑

- 自动保存目录落地：`图片目录/轻影截图`
- 文件命名规则：`<场景标签>_<时间戳>.<扩展名>`，冲突自动重命名
- 全屏截图：复制到剪贴板后自动保存（受自动保存开关控制）
- 区域截图：仅工具条“保存”动作触发自动保存（受自动保存开关控制）
- 自动保存开关通过 `QSettings` 持久化

### T206 提示系统重构与托盘隐藏一致性

- 新增 `TipPresenter`，提示默认采用“鼠标优先、越界回退右下角”定位策略
- 新增 `CaptureUiStateCoordinator`，统一维护托盘隐藏与截图会话恢复策略
- 新增 `CaptureResultHandler`，统一输出截图动作决策（窗口恢复 / 手动保存）
- 托盘隐藏后截图完成不再拉起主窗口
- 托盘隐藏状态下手动保存仅弹保存对话框，不恢复主窗口

### T207 QSettings 品牌化命名与旧配置迁移

- 新命名空间切换为 `QingYing/LightShadowCapture`
- 新增 `SettingsService` 统一配置读写入口
- 首次启动自动迁移 `shanzhi/screenCapture` 旧配置
- 迁移策略：新值优先，不覆盖新命名空间已有配置
- 写入迁移标记：`meta/settings_migrated_from_legacy`

### T208 区域截图右键取消 + 可配置热键 + 自动保存目录

- 区域截图支持鼠标右键取消（等价 ESC / 工具条 ✕）
- 新增 `GlobalHotkeyManager`，负责全局热键注册、更新、回滚与激活分发
- 新增 `CaptureSettingsDialog`，可配置并持久化截图热键与自动保存目录
- 新增配置键：`capture/hotkey_sequence`、`capture/auto_save_dir`
- 热键变更后立即生效；注册失败保留上一次可用热键并提示原因
- 自动保存 ON 时优先写入用户目录，不可写时回退默认目录并提示

### T401 Windows 长截图重构立项（实验接入）

- 新增长截图实验开关：`SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE`
- 默认关闭：关闭时不编译长截图模块，不显示长截图入口
- 已接入实验性长截图基础架构：
  - `LongCaptureSessionController`
  - `LongCaptureSession`
  - `LongCaptureBackend` / `DesktopGdiCaptureBackend`
  - `ScrollDispatcher`
  - `MotionDetector`
  - `StableFrameCollector`
  - `OverlapMatcher`
  - `StitchComposer`
  - `LongCapturePreviewPanel`
- 当前状态：已完成 Windows GDI V1 架构接回、Overlay/UI 挂接与手动滚轮主路径打通；自动滚动已移除，默认隐藏，不视为验收通过

### T402 手动长截图恢复（实验链路修正）

- 长截图恢复为“仅手动滚轮”模式，自动滚动继续移除
- 滚动目标解析仅在首次进入或目标失效时短暂隐藏 Overlay 一次
- 每次滚轮观测与抓帧不再反复 hide/show Overlay，避免持续闪烁
- 删除基于 `GW_HWNDNEXT` 的下层窗口猜测逻辑，回到按点解析真实滚动目标
- 当前状态：优先保证实验手动长截图可用，仍默认关闭，不视为验收通过

## 当前状态

### 标准截图主线

- 全屏截图：可用
- 区域截图：可用
- 保存 / 复制 / 自动保存：可用
- 热键 / 托盘 / 历史记录 / 设置：可用

### 长截图

- 旧版已回退
- 新版已按 T401/T402 以实验特性形式重新接入
- 当前仅保留手动滚轮长截图，自动滚动不再提供
- 默认关闭，不对外开放入口
- 仅在打开 `SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE` 后参与构建与运行

### 标注工具

- T301 画笔：已完成，支持区域截图 / 长截图内自由绘制、颜色锁定与结果合成
- T302 矩形：已完成，支持区域截图 / 长截图内拖拽绘制、颜色复用与结果合成
- T303 圆形：已完成，支持区域截图 / 长截图内拖拽绘制、颜色复用与结果合成
- T304 文字：已完成，支持区域截图 / 长截图内单击输入、回车提交、颜色复用与结果合成

## 待办（P1）

### T305 马赛克工具实现
### T306 撤销/重做
### T403 WGC/DXGI 后端升级

## 说明

- 当前稳定发布路径仍是“全屏截图 / 区域截图”。
- T401/T402 的长截图实现已拆成独立实验链路，避免继续污染标准截图主流程。
- 新增功能优先服务化拆分，避免向 `MainWindow` 持续堆积多职责流程细节。

