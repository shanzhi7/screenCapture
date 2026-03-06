# 按钮映射清单（Button Map）

> 更新时间：2026-03-06
> 目的：将界面按钮与当前实现状态、后续任务编号一一对应，便于按图索骥开发。

---

## 1 主窗口按钮映射（MainWindow）

| 区域 | objectName | 文案/图标 | 当前状态 | 对应任务 |
| --- | --- | --- | --- | --- |
| 左侧导航 | `sideCaptureButton` | 相机图标 | 占位提示 | `T109` |
| 左侧导航 | `sideGalleryButton` | 文件夹图标 | 占位提示 | `T110` |
| 左侧导航 | `sideConfigButton` | 设置图标 | 占位提示 | `T111` |
| 左侧导航 | `sideBottomButton` | 设置图标 | 占位提示 | `T112` |
| 顶部操作 | `btnPrev` | 左箭头 | 占位提示 | `T113` |
| 顶部操作 | `btnShotIcon` | 截图图标 | 占位提示 | `T114` |
| 顶部操作 | `btnNext` | 右箭头 | 占位提示 | `T115` |
| 顶部操作 | `btnNewTask` | `＋ 新建` | 占位提示 | `T108` |
| 顶部操作 | `btnHistory` | 历史图标 | 占位提示 | `T102` |
| 顶部操作 | `btnTopSettings` | `设置` | 占位提示 | `T116` |
| 快速截图 | `modeFullButton` | `全屏` | 已实现 | 已完成 |
| 快速截图 | `modeRegionButton` | `区域` | 已实现 | 已完成 |
| 快速截图 | `modeScrollButton` | `滚动` | 占位（待实现） | `T101` |
| 快速截图 | `startCaptureButton` | `开始截图/重新截图` | 已实现 | 已完成 |
| 最近截图 | `btnMoreRecent` | `更多 >` | 占位提示 | `T103` |
| 右侧格式 | `btnMoreFormat` | `更多 >` | 占位提示 | `T104` |
| 常用设置 | `btnHotkeySetting` | `热键 Ctrl+Shift+A` | 占位提示 | `T105` |
| 常用设置 | `btnFormatSetting` | `格式 PNG` | 占位提示 | `T106` |
| 常用设置 | `btnAutoSave` | `自动保存 ON/OFF` | 文案切换已实现，真实逻辑待接入 | `T107` |

---

## 2 覆盖层工具条映射（SelectionOverlay）

> 说明：以下按钮位于截图遮罩层 `SelectionOverlay` 内部工具条，未在 `mainwindow.ui` 里声明 objectName。

| 模块 | 变量名 | 文案 | 当前状态 | 对应任务 |
| --- | --- | --- | --- | --- |
| 编辑工具 | `btnPen` | `画笔` | 模式按钮（UI态） | `T201` |
| 编辑工具 | `btnRect` | `矩形` | 模式按钮（UI态） | `T202` |
| 编辑工具 | `btnEllipse` | `圆形` | 模式按钮（UI态） | `T203` |
| 编辑工具 | `btnText` | `文字` | 模式按钮（UI态） | `T204` |
| 编辑工具 | `btnMosaic` | `马赛克` | 模式按钮（UI态） | `T205` |
| 操作按钮 | `m_btnCopy` | `复制` | 已接入复制流程 | 已完成 |
| 操作按钮 | `m_btnSave` | `保存` | 已接入保存流程 | 已完成 |
| 操作按钮 | `m_btnCancel` | `✕` | 已接入取消流程 | 已完成 |
| 操作按钮 | `m_btnConfirm` | `✓` | 已接入确认流程 | 已完成 |

---

## 3 状态定义说明

- `已实现`：点击后执行真实业务逻辑。
- `占位提示`：点击后通过 `ShowTip` 提示“后续实现”，无真实业务。
- `模式按钮（UI态）`：按钮可选中，但业务引擎未接入。

---

## 4 建议开发顺序（基于按钮）

1. `T101` 滚动截图（快速截图核心缺口）
2. `T102` + `T103` 历史入口与最近截图数据化
3. `T105` + `T106` + `T107` 常用设置接入
4. `T201~T206` 覆盖层编辑引擎与撤销重做
5. `T108~T116` 导航/顶部占位按钮业务化

---

## 5 文档联动

- 任务编号来源：`doc/05_codex_tasks.md`
- UI结构来源：`doc/06_ui_design.md`
- 功能范围来源：`doc/07_feature_design.md`
