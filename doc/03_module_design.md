# 模块设计（同步版）

> 更新时间：2026-03-06

## 1 MainWindow 模块

### 职责

- 主窗口初始化与界面状态维护
- 截图模式切换
- 调度全屏/区域截图
- 预览更新、保存、复制
- 未实现按钮占位处理

### 关键接口

- `void startCapture();`
- `void onModeFullClicked();`
- `void onModeRegionClicked();`
- `void onModeScrollClicked();`
- `void onSelectionFinished(const QRect &rect);`
- `void onSelectionCanceled();`
- `void onOverlayCopyRequested(const QRect &rect);`
- `void onOverlaySaveRequested(const QRect &rect);`
- `void saveCurrentImage();`
- `void copyCurrentImage();`
- `QPixmap captureRegion(const QRect &rect) const;`
- `void updatePreview(const QPixmap &pixmap);`
- `void showTip(const QString &text);`

### 备注

- 当前抓图函数仍在 MainWindow 内，后续建议拆分到服务层。

---

## 2 SelectionOverlay 模块

### 职责

- 全屏遮罩显示
- 鼠标拖拽选区
- 选区绘制与尺寸提示
- 工具条展示与按钮事件发射

### 关键接口/事件

- `mousePressEvent`
- `mouseMoveEvent`
- `mouseReleaseEvent`
- `keyPressEvent`
- `paintEvent`
- `QRect currentRect() const;`
- `void updateToolbarPosition();`
- `void ensureToolbar();`
- `void confirmSelection();`

### 信号

- `selectionFinished(const QRect &rect)`
- `selectionCanceled()`
- `copyRequested(const QRect &rect)`
- `saveRequested(const QRect &rect)`

---

## 3 ShowTip 模块

### 职责

- 显示短时浮动消息
- 统一操作反馈体验

### 关键接口

- `void showText(const QString &text, QWidget *anchor, int timeoutMs = 1800);`

---

## 4 样式与资源模块

### 样式

- `styles/hyperos.qss`
- 负责布局细节、控件状态、交互视觉反馈

### 图标资源

- `icons/*.svg`
- 通过资源系统打包并在按钮上加载

---

## 5 近期模块演进建议

1. 新增 `ScreenCaptureService`：剥离抓图逻辑。
2. 新增 `HistoryService`：历史数据管理与缩略图缓存。
3. 新增 `SettingsService`：热键、格式、自动保存持久化。
4. 新增 `AnnotationEngine`：覆盖层编辑工具真实绘制能力。
