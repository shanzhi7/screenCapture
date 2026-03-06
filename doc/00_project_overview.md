# Qt ScreenCapture Tool

## 项目简介

本项目是一个基于 **Qt Widgets + CMake + C++17** 的桌面截图工具，当前已完成主界面重构与基础截图闭环。

目标是开发一个 **轻量、好用、响应快速、界面现代化** 的截图工具，用于日常开发与学习。

---

## 技术栈

- C++17
- Qt6 Widgets（已使用 `Qt::Core`、`Qt::Widgets`、`Qt::Svg`）
- CMake
- Windows

---

## 当前目录结构（核心）

```text
screenCapture
├─ main.cpp
├─ mainwindow.h/.cpp/.ui
├─ selectionoverlay.h/.cpp
├─ showtip.h/.cpp/.ui
├─ styles/hyperos.qss
├─ icons/*.svg
├─ resources.qrc
├─ CMakeLists.txt
└─ doc/
```

---

## 当前进度快照（2026-03-06）

### 已实现

- 主界面重构为 HyperOS 风格（卡片化布局、图标按钮、QSS 统一主题）。
- 截图模式分段按钮：`全屏 / 区域 / 滚动(占位)`。
- 区域截图遮罩层 `SelectionOverlay`：
  - 半透明遮罩
  - 拖拽选区
  - 选区尺寸提示
  - ESC 取消、Enter 确认
  - 选区后悬浮工具条（画笔/矩形/圆形/马赛克/文字/复制/保存/取消/确认）
- 全屏截图、区域截图、保存 PNG、复制到剪贴板。
- 提示组件 `ShowTip` 已替代 `showMessage`。

### 已搭建但未完成功能

- 滚动截图（UI 入口已存在，逻辑待实现）
- 主界面多处按钮为占位接口（详见 `05_codex_tasks.md`）
- 覆盖层中的标注工具按钮尚未接入真实绘制引擎

---

## 版本目标（更新）

### v1.0（当前基础可用）

- 全屏截图
- 区域截图
- 预览显示
- 保存 PNG
- 复制剪贴板
- 现代化主界面

### v1.1（近期）

- 滚动截图基础能力
- 最近截图真实数据化
- 常用设置三项功能接入（热键/格式/自动保存）

### v2.0（中期）

- 标注绘制（画笔/矩形/圆形/文字）
- 马赛克
- 撤销/重做
- 截图后编辑工作流完善

### v3.0（后续）

- 托盘与全局快捷键
- 贴图模式
- OCR 与历史管理增强
