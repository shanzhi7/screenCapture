# LightShadowCapture / 轻影截图

> A lightweight Windows screenshot tool built with Qt Widgets.
> 基于 Qt Widgets 的轻量级 Windows 截图工具。

## Download / 下载

- GitHub Releases: `https://github.com/<your-org-or-name>/<your-repo>/releases`
- ZIP package: `https://github.com/<your-org-or-name>/<your-repo>/releases/download/<tag>/LightShadowCapture-portable.zip`
- EXE installer: `https://github.com/<your-org-or-name>/<your-repo>/releases/download/<tag>/LightShadowCapture-setup.exe`

Replace the placeholder links above after publishing the repository and release packages.
Please replace `<tag>` with your actual release tag, such as `v0.2.

## Overview / 项目简介

**English**

LightShadowCapture is a desktop screenshot tool focused on fast everyday capture, simple annotation, and a clean main window workflow. It supports full-screen capture, region capture, annotation, history preview, auto save, drag-to-pin, and always-on-top pinned images.

**中文**

轻影截图是一个面向日常使用的桌面截图工具，重点在于快速截图、轻量标注和清晰的主界面工作流。当前支持全屏截图、区域截图、标注编辑、历史预览、自动保存、拖拽贴图和桌面置顶贴图。

## Features / 功能特性

### Capture / 截图能力

- Full-screen capture with multi-monitor selection
- Region capture with overlay toolbar
- Experimental long screenshot flow in region mode
- Copy capture result directly to clipboard

- 支持全屏截图，并可在多显示器环境下选择目标屏幕
- 支持区域截图，并提供截图工具条
- 支持区域模式下的实验性长截图流程
- 支持将截图结果直接复制到剪贴板

### Annotation / 标注能力

- Pen, rectangle, ellipse, mosaic, and text tools
- Undo and redo support
- Save, confirm, cancel, and pin actions from the overlay toolbar

- 支持画笔、矩形、圆形、马赛克、文字标注
- 支持撤销与重做
- 支持在截图工具条中执行保存、确认、取消和贴图

### History & Preview / 历史与预览

- Recent captures on the left side of the main window
- Full gallery page for browsing local capture history
- Previous / latest / next navigation in history context
- Drag a recent capture out of the main window to create a pinned image

- 主界面左侧提供最近截图区域
- 提供独立图库页浏览本地截图历史
- 支持上一张 / 最新 / 下一张历史导航
- 支持将最近截图直接拖出主窗口生成贴图

### Pinned Images / 贴图能力

- Always-on-top pinned image windows
- Drag to move
- Mouse wheel zoom
- Minimize, maximize/restore, and close controls
- Multiple pinned windows can exist at the same time

- 支持桌面置顶贴图窗口
- 支持拖动移动
- 支持鼠标滚轮缩放
- 支持最小化、最大化/还原、关闭
- 支持同时保留多个贴图窗口

### Settings / 设置能力

- Auto save switch
- Custom auto save directory
- Output format switch: PNG / JPG
- Global screenshot hotkey
- Launch at startup option for Windows login

- 支持自动保存开关
- 支持自定义自动保存目录
- 支持 PNG / JPG 输出格式切换
- 支持全局截图热键
- 支持 Windows 登录后开机自启动

## Platform / 适用平台

**English**

- Primary target: Windows
- UI framework: Qt 6 Widgets
- Current repository is optimized for local desktop usage and packaging on Windows

**中文**

- 当前主要面向 Windows 平台
- 使用 Qt 6 Widgets 构建界面
- 当前仓库主要针对 Windows 桌面环境开发和打包

## Quick Start / 快速开始

### For End Users / 普通用户

**English**

1. Download the packaged release from the release page.
2. Extract the package or run the installer.
3. Launch `screenCapture.exe`.
4. Configure hotkey, auto save directory, and startup behavior in Settings if needed.

**中文**

1. 从发布页下载打包好的程序。
2. 解压压缩包或运行安装程序。
3. 启动 `screenCapture.exe`。
4. 如有需要，可在设置中调整热键、自动保存目录和开机自启动。

### Basic Workflow / 基本使用流程

**English**

1. Choose full-screen mode or region mode in the main window.
2. Start capture.
3. Copy, save, pin, or continue browsing history from the result.

**中文**

1. 在主界面选择全屏模式或区域模式。
2. 点击开始截图。
3. 根据需要复制、保存、贴图，或继续查看历史截图。

## Build From Source / 从源码构建

### Requirements / 环境要求

- CMake 3.19 or newer
- Qt 6.5 or newer
- C++17 compiler
- Windows build environment recommended

### Build Steps / 构建步骤

```bash
cmake -S . -B build
cmake --build build --config Release
```

**English**

The project uses Qt Core, Widgets, and Svg. Experimental long capture is enabled by default in the repository build configuration on Windows.

**中文**

项目依赖 Qt Core、Widgets、Svg。当前仓库默认在 Windows 构建中开启实验性长截图能力。

## Notes / 使用说明

### Auto Save vs History / 自动保存与历史缓存

**English**

Auto save directory and capture history are different concepts:

- Auto save writes files to the directory configured by the user.
- Capture history is stored locally for recent preview and gallery browsing.

**中文**

自动保存目录和历史缓存是两套不同机制：

- 自动保存会把文件写入用户设置的保存目录。
- 截图历史会本地缓存，用于最近截图和图库浏览。

### Long Screenshot / 长截图

**English**

Long screenshot is currently an experimental feature. It is already available in the repository, but complex scrolling scenes may still need further tuning.

**中文**

长截图当前属于实验性功能。仓库中已经接入，但在复杂滚动场景下仍可能继续优化。

## Project Docs / 项目文档

For development notes and module documentation, see the [`doc/`](doc/) directory.
开发说明和模块文档请查看 [`doc/`](doc/) 目录。

Recommended files / 推荐先阅读：

- [`doc/00_project_overview.md`](doc/00_project_overview.md)

- [`doc/03_module_design.md`](doc/03_module_design.md)

- [`doc/08_button_map.md`](doc/08_button_map.md)

  

## License / 许可证

This project is licensed under the MIT License. See the [`LICENSE`](LICENSE) file for details.
本项目采用 MIT License，详见 [`LICENSE`](LICENSE) 文件。