# 项目总览（同步版）

> 更新时间：2026-03-07

## 1. 项目目标

轻影截图是一个基于 Qt Widgets 的 Windows 桌面截图工具，目标是：

- 提供稳定的全屏截图与区域截图能力
- 在区域截图中提供可用的长截图流程
- 采用简约高级的 HyperOS 风格 UI，保持高可读性与低学习成本
- 为后续历史、设置、编辑能力预留清晰扩展点

## 2. 当前技术栈

- C++17
- Qt 6.5（Core / Widgets / Svg）
- CMake
- Windows 优先（包含 Win32 全局热键注册）

## 3. 当前目录（核心）

```text
screenCapture/
├─ main.cpp
├─ mainwindow.h/.cpp/.ui
├─ selectionoverlay.h/.cpp
├─ longcapturestitcher.h/.cpp
├─ capturehistorymanager.h/.cpp
├─ settingsservice.h/.cpp
├─ showtip.h/.cpp/.ui
├─ closedialog.h/.cpp/.ui
├─ styles/hyperos.qss
├─ icons/*.svg
├─ icon.ico
├─ appicon.rc
├─ CMakeLists.txt
└─ doc/
```

## 4. 当前功能状态

### 已实现

- 主界面重构（非透明主窗口、卡片式布局、QSS 统一主题）
- 全屏截图（多屏环境下可选择目标显示器）
- 全屏截图结果复制到剪贴板
- 区域截图（拖拽选区、确认复制、保存文件）
- 区域模式内长截图（滚轮采帧 + 增量拼接 + 复制/保存）
- 跨屏区域抓图合成
- 历史记录第一阶段（本地持久化 + 最近截图真实缩略图展示）
- 输出格式配置（PNG/JPG 切换 + 本地持久化）
- 自动保存真实逻辑（目录/命名/冲突重命名/开关持久化）
- 配置命名升级与迁移（`QingYing/LightShadowCapture`，兼容 `shanzhi/screenCapture`）
- 全局热键截图（Windows：`Ctrl+Shift+A`）
- 系统托盘（显示主界面 / 退出应用）
- 关闭主窗口时弹出 `CloseDialog`（隐藏到托盘 / 退出 / 取消）
- 统一提示系统 `ShowTip`
- 应用图标接入（窗口图标 + 可执行文件图标）

### 部分实现

- Overlay 编辑工具按钮已具备 UI 入口（画笔/矩形/圆形/马赛克/文字），绘制引擎未接入
- 历史高级能力（分页、删除、筛选）未实现

### 待实现

- 热键配置持久化与冲突检测
- 编辑引擎、撤销重做、标注能力
