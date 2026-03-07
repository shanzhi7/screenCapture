# 项目总览（同步版）

> 更新时间：2026-03-06

## 1. 项目目标

轻影截图是一个基于 Qt Widgets 的桌面截图工具，目标是提供：

- 快速稳定的截图流程（全屏、区域、长截图）
- 现代化主界面（HyperOS 风格）
- 可扩展的功能架构（历史、设置、编辑工具）

## 2. 当前技术栈

- C++17
- Qt6（Core / Widgets / Svg）
- CMake
- Windows 优先

## 3. 当前目录（核心）

```text
screenCapture/
├─ main.cpp
├─ mainwindow.h/.cpp/.ui
├─ selectionoverlay.h/.cpp
├─ longcapturestitcher.h/.cpp
├─ showtip.h/.cpp/.ui
├─ styles/hyperos.qss
├─ icons/*.svg
├─ resources.qrc
├─ CMakeLists.txt
└─ doc/
```

## 4. 当前功能状态

### 已实现

- 主界面重构与 QSS 主题（HyperOS 风格）
- 全屏截图（支持多屏选择目标显示器）
- 区域截图（拖拽选区、确认/取消、保存）
- 区域截图确认后自动复制到剪贴板
- 区域模式内长截图（工具条“长截图”，滚轮触发多帧拼接）
- 长截图结果保存 / 复制
- `ShowTip` 统一提示系统

### 部分实现

- 长截图稳定性已显著改善（已引入 `LongCaptureStitcher` 增量拼接），仍需持续优化极端场景
- 覆盖层编辑工具按钮已具备 UI 入口（画笔/矩形/圆形/马赛克/文字），绘制引擎未接入

### 待实现

- 历史数据持久化与历史面板
- 设置项持久化（热键、格式、自动保存目录）
- 完整编辑引擎与撤销重做
