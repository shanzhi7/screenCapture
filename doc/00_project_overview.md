# Qt ScreenCapture Tool

## 项目简介

这是一个基于 **Qt Widgets + CMake + C++17** 的桌面截图工具。

目标是开发一个 **轻量、好用、响应快速的截图工具**，主要用于日常开发和学习使用。

核心功能包括：

- 全屏截图
- 区域截图
- 截图预览
- 保存图片
- 复制到剪贴板

项目目标：

1. 提供一个简单易用的截图工具
2. 熟悉 Qt 桌面开发
3. 构建一个结构清晰的 C++ 桌面项目
4. 通过 AI（Codex）协助开发，提高效率

---

## 技术栈

- C++17
- Qt6 / Qt5 Widgets
- CMake
- Windows

主要 Qt 模块：

- QtWidgets
- QtGui
- QtCore

---

## 目前项目目录结构

screenCapture
 ├─ build
 ├─ main.cpp
 ├─ mainwindow.h
 ├─ mainwindow.cpp
 ├─ mainwindow.ui
 ├─ CMakeLists.txt
 └─ doc

---

## 项目目标版本

### v1.0

最小可用版本：

- 区域截图
- 预览
- 保存
- 复制

### v1.1

体验优化：

- Esc取消
- Enter确认
- 尺寸提示

### v2.0

高级功能：

- 标注
- 托盘
- 快捷键
- 截图未保存时可以进行各种图画，马赛克等操作