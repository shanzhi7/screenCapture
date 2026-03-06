# 模块设计

## MainWindow

职责：

- 启动截图
- 显示截图
- 保存图片

主要函数：

void startCapture();
 void showPreview(QPixmap img);
 void saveImage();
 void copyImage();

---

## SelectionOverlay

全屏透明窗口

功能：

- 捕获鼠标
- 绘制选择框

关键事件：

mousePressEvent
 mouseMoveEvent
 mouseReleaseEvent

---

## ScreenCapture

功能：

QPixmap captureRegion(QRect rect);

实现：

QScreen::grabWindow()

---

## ImageManager

功能：

saveImage()
 copyToClipboard()