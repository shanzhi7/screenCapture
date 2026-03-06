# 功能设计文档

本项目包含以下核心功能：

1. 区域截图
2. 截图预览
3. 保存图片
4. 复制图片
5. 取消截图

---

# 1 区域截图

## 功能描述

用户可以拖动鼠标选择屏幕区域进行截图。

---

## 功能流程

用户点击 Capture
 ↓
 打开 SelectionOverlay
 ↓
 鼠标拖动选择区域
 ↓
 生成 QRect
 ↓
 调用截图接口
 ↓
 生成 QPixmap

\## 技术实现

截图接口：QScreen::grabWindow()

示例：

QScreen *screen = QGuiApplication::primaryScreen();
 QPixmap pix = screen->grabWindow(0, x, y, width, height);

---

# 2 截图预览

截图完成后：

MainWindow 显示图片。

组件：QLabel

实现：previewLabel->setPixmap(image);

---

# 3 保存图片

用户点击 Save。

流程：

点击Save
 ↓
 打开 QFileDialog
 ↓
 选择路径
 ↓
 保存PNG

示例：pixmap.save("screenshot.png");

---

# 4 复制图片

用户点击 Copy。

流程：

点击Copy
 ↓
 获取系统剪贴板
 ↓
 复制QPixmap

示例：

QClipboard *clipboard = QApplication::clipboard();
 clipboard->setPixmap(pixmap);

---

# 5 取消截图

用户按 ESC。

行为：

关闭 SelectionOverlay
 返回 MainWindow

\---

\# 6 错误处理

如果用户未选择区域：不执行截图

如果截图失败：弹出 QMessageBox

---

# 7 性能设计

截图延迟：< 100ms

UI响应：< 16ms

---

# 8 扩展功能（未来版本）

未来计划：

### v2

- 标注工具
- 箭头
- 文本
- 马赛克

### v3

- OCR识别
- 滚动截图
- 贴图模式