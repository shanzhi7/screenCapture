# 系统架构（同步版）

> 更新时间：2026-03-06

## 1 总体分层

系统采用四层组织：

1. UI Layer（主界面）
2. Capture Layer（截图与选区）
3. Feedback Layer（提示反馈）
4. Resource/Style Layer（资源与主题）

---

## 2 UI Layer

### MainWindow

职责：

- 承载主界面布局与按钮入口
- 管理截图模式（全屏/区域/滚动）
- 调度截图流程与结果预览
- 承接占位按钮统一提示

关键对象：

- `mainwindow.ui`
- `mainwindow.h/.cpp`

---

## 3 Capture Layer

### SelectionOverlay

职责：

- 全屏遮罩选区
- 鼠标交互与选区绘制
- 工具条定位与确认/取消/保存/复制触发

关键对象：

- `selectionoverlay.h/.cpp`

### Screen Capture（当前内聚于 MainWindow）

- 通过 `QScreen::grabWindow` 执行抓取
- 后续建议提取为 `ScreenCaptureService`

---

## 4 Feedback Layer

### ShowTip

职责：

- 提供统一短时提示
- 取代 `showMessage/statusBar` 的分散提示

关键对象：

- `showtip.h/.cpp/.ui`

---

## 5 Resource/Style Layer

### QSS 主题

- 文件：`styles/hyperos.qss`
- 负责统一视觉风格、按钮状态、卡片样式

### SVG 图标资源

- 文件：`icons/*.svg`
- 通过 CMake 资源系统打包进可执行文件

### CMake 资源打包

- 使用 `qt_add_resources` 显式打包样式与图标
- 链接 `Qt::Svg` 以支持 SVG 渲染

---

## 6 当前架构结论

已形成“可运行 + 可扩展”的基础架构：

- UI 与截图交互已闭环
- 提示与样式已统一
- 下一阶段重点是把占位入口接入真实业务（滚动截图、历史、设置、编辑引擎）
