```
# Codex开发任务拆分

本文件用于指导 Codex 按模块开发项目。

开发流程遵循：

模块开发 → 集成 → 测试 → 体验优化

---

# Phase 1 基础截图功能（v1.0）

目标：

实现最小可用截图工具。

---

## Task 1 实现 SelectionOverlay

实现截图遮罩层。

要求：

- 创建一个全屏透明 QWidget
- 遮罩颜色为半透明黑色
- 支持鼠标拖动选择区域
- 绘制矩形选择框

技术要点：
```

mousePressEvent
 mouseMoveEvent
 mouseReleaseEvent
 paintEvent

```
实现逻辑：
```

鼠标按下 → 记录起点
 鼠标移动 → 更新矩形
 鼠标释放 → 确定选区

```
输出：
```

QRect selectedRect

```
---

## Task 2 实现 ScreenCapture

实现截图模块。

要求：

使用 Qt API：
```

QScreen::grabWindow()

```
实现：
```

QPixmap captureRegion(QRect rect)

```
示例：
```

QScreen *screen = QGuiApplication::primaryScreen();
 QPixmap pixmap = screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());

```
---

## Task 3 MainWindow 集成截图

流程：
```

用户点击 Capture
 ↓
 创建 SelectionOverlay
 ↓
 用户选择区域
 ↓
 获取 QRect
 ↓
 调用 ScreenCapture
 ↓
 返回 QPixmap

```
MainWindow 负责：

- 启动截图
- 接收截图结果
- 更新UI

---

## Task 4 实现截图预览

在 MainWindow 中显示截图。

使用：
```

QLabel

```
实现：
```

previewLabel->setPixmap(image);

```
要求：

- 图片自动缩放
- 保持比例

---

## Task 5 实现保存图片

用户点击 Save。

实现：
```

QFileDialog

```
流程：
```

点击 Save
 ↓
 打开文件选择窗口
 ↓
 选择路径
 ↓
 保存 PNG

```
代码示例：
```

pixmap.save(path);

```
---

## Task 6 实现复制图片

用户点击 Copy。

使用：
```

QClipboard

```
实现：
```

QClipboard *clipboard = QApplication::clipboard();
 clipboard->setPixmap(pixmap);

```
---

# Phase 2 体验优化（v1.1）

目标：

优化截图体验。

---

## Task 7 ESC取消截图

行为：
```

ESC → 关闭 SelectionOverlay

```
要求：

- 不生成截图
- 返回主界面

实现：
```

keyPressEvent

```
---

## Task 8 Enter确认截图

行为：
```

Enter → 确认当前选区

```
流程：
```

获取 QRect
 执行截图
 关闭遮罩

```
---

## Task 9 显示选区尺寸

截图时显示：
```

width x height

```
例如：
```

800 x 600

```
显示位置：
```

选区右下角

```
实现：
```

QPainter 绘制文本

```
---

## Task 10 选区高亮效果

截图界面：

- 背景半透明黑色
- 选区保持原亮度

实现方式：
```

QPainter 遮罩绘制

```
---

# Phase 3 UI增强（v1.2）

目标：

提升界面体验。

---

## Task 11 支持移动选区

用户可以：

拖动已选区域。

行为：
```

鼠标拖动选区

```
---

## Task 12 支持调整选区大小

用户可以：

拖动选区边缘调整大小。

实现：
```

检测鼠标位置
 改变 cursor

```
---

## Task 13 添加工具栏

在主窗口增加：
```

Capture
 Save
 Copy

```
可扩展：
```

Redo
 Clear

```
---

# Phase 4 高级功能（v2.0）

目标：

实现截图工具高级功能。

---

## Task 14 添加标注工具

截图完成后进入标注界面。

支持：

- 矩形
- 箭头
- 文字

实现：
```

QPainter

```
工具：
```

AnnotationWidget

```
---

## Task 15 添加马赛克工具

功能：

隐藏敏感信息。

实现：
```

图像像素化

```
---

## Task 16 添加画笔工具

支持：
```

自由绘制

```
---

## Task 17 添加撤销功能

支持：
```

Ctrl + Z

```
实现：
```

操作栈

```
---

# Phase 5 系统功能（v3.0）

目标：

提高软件实用性。

---

## Task 18 添加系统托盘

程序最小化到托盘。

使用：
```

QSystemTrayIcon

```
功能：
```

右键菜单

```
---

## Task 19 添加全局快捷键

支持：
```

Ctrl + Shift + A

```
触发截图。

Windows实现：
```

RegisterHotKey

```
---

## Task 20 贴图模式

截图后：
```

悬浮窗口

```
类似：
```

Snipaste

```
特点：

- 永远置顶
- 可拖动

---

# Phase 6 高级扩展（未来）

## Task 21 OCR识别

识别截图文字。

可使用：
```

Tesseract OCR

```
---

## Task 22 滚动截图

支持：
```

网页长截图

```
---

## Task 23 截图历史

记录最近截图。

---

# Codex执行规则

Codex开发必须：

1. 每次只实现一个Task
2. 代码必须符合架构文档
3. UI设计遵循 ui_design.md
4. 功能遵循 feature_design.md
5. 不得修改无关模块
```

