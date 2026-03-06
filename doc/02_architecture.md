# 系统架构

系统分为三个主要模块：

UI Layer
 │
 ├── MainWindow
 │
 Capture Layer
 │
 ├── ScreenCapture
 ├── SelectionOverlay
 │
 Core Layer
 │
 ├── ImageManager

---

## 1 UI Layer

负责用户界面：

MainWindow：

- 按钮
- 预览
- 保存

---

## 2 Capture Layer

负责截图逻辑：

ScreenCapture

功能：

- 调用 QScreen
- 获取截图

SelectionOverlay

功能：

- 全屏遮罩
- 鼠标拖动
- 绘制选择框

---

## 3 Core Layer

ImageManager

负责：

- 图片保存
- 剪贴板