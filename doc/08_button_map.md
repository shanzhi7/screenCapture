# 按钮映射（同步版）

> 更新时间：2026-03-14

## 1. MainWindow 主界面按钮

| objectName | 按钮文案/入口 | 实际行为 | 状态 |
| --- | --- | --- | --- |
| `modeFullButton` | 全屏 | 切换到全屏截图模式 | 已实现 |
| `modeRegionButton` | 区域 | 切换到区域截图模式 | 已实现 |
| `modeScrollButton` | 滚动 | 独立滚动模式入口，当前界面隐藏 | 已隐藏 |
| `startCaptureButton` | 开始截图 | 按当前模式启动截图 | 已实现 |
| `btnTopSettings` | 顶部设置 | 打开截图设置弹窗 | 已实现 |
| `sideConfigButton` | 侧栏设置 | 打开截图设置弹窗 | 已实现 |
| `btnHotkeySetting` | 热键设置 | 打开截图设置弹窗 | 已实现 |
| `sideBottomButton` | 关于 | 打开关于弹窗 | 已实现 |
| `btnAutoSave` | 自动保存 | 切换自动保存开关并持久化 | 已实现 |
| `btnFormatSetting` | 格式 PNG/JPG | 切换输出格式 | 已实现 |
| `btnMoreFormat` | 更多格式 | 当前直接切换输出格式 | 已实现（简化行为） |
| `btnHistory` | 历史 | 刷新最近截图列表并提示数量 | 已实现（简化行为） |
| `btnMoreRecent` | 更多最近截图 | 当前直接刷新最近截图列表 | 已实现（简化行为） |
| `sideCaptureButton` | 侧栏拍摄 | 仅提示“功能预留” | 未实现 |
| `sideGalleryButton` | 侧栏图库 | 仅提示“功能预留” | 未实现 |
| `btnPrev` | 上一张 | 仅提示“功能预留” | 未实现 |
| `btnShotIcon` | 中间截图图标 | 仅提示“功能预留” | 未实现 |
| `btnNext` | 下一张 | 仅提示“功能预留” | 未实现 |
| `btnNewTask` | 新建 | 仅提示“功能预留” | 未实现 |

## 2. 最近截图区域

| objectName | 按钮文案/入口 | 实际行为 | 状态 |
| --- | --- | --- | --- |
| `recentThumbButton` | 历史缩略图 | 加载该历史图片到主预览区 | 已实现 |

## 3. SelectionOverlay 主工具条

| 按钮/交互 | 实际行为 | 状态 |
| --- | --- | --- |
| 画笔 | 自由绘制，支持颜色/粗细子工具条 | 已实现 |
| 矩形 | 拖拽绘制矩形，支持颜色/粗细子工具条 | 已实现 |
| 圆形 | 拖拽绘制圆形，支持颜色/粗细子工具条 | 已实现 |
| 马赛克 | 自由涂抹打码，支持粗细子工具条 | 已实现 |
| 文字 | 单击输入文字，支持颜色/字号子工具条 | 已实现 |
| 撤销 | 回退最近一次标注 | 已实现 |
| 重做 | 恢复最近一次撤销的标注 | 已实现 |
| 长截图 | 进入/退出长截图状态 | 已实现（实验功能） |
| 保存 | 普通区域截图保存 / 长截图保存 | 已实现 |
| 取消（✕） | 取消当前截图 | 已实现 |
| 确认（✓） | 普通截图复制 / 长截图复制 | 已实现 |
| 再次点击已选工具 | 取消当前工具选中 | 已实现 |
| 鼠标右键 | 取消当前截图 | 已实现 |
| `Esc` | 取消当前截图或取消当前文字输入 | 已实现 |
| `Enter` | 确认当前截图 / 提交当前文字输入 | 已实现 |
| `Ctrl+Z` | 撤销 | 已实现 |
| `Ctrl+Y` | 重做 | 已实现 |

## 4. SelectionOverlay 子工具条

| 子工具条项 | 实际行为 | 状态 |
| --- | --- | --- |
| 粗细按钮（细/中/粗） | 为当前编辑工具切换粗细 | 已实现 |
| 颜色按钮 | 为画笔/矩形/圆形/文字切换颜色 | 已实现 |
| 马赛克颜色按钮 | 仅保留统一样式，不参与渲染 | 已实现（禁用显示） |

## 5. 设置与弹窗按钮

| 对话框 / objectName | 按钮文案 | 实际行为 | 状态 |
| --- | --- | --- | --- |
| `CaptureSettingsDialog.resetHotkeyButton` | 恢复默认 | 恢复默认截图热键 | 已实现 |
| `CaptureSettingsDialog.browseDirButton` | 浏览 | 选择自动保存目录 | 已实现 |
| `CaptureSettingsDialog.clearDirButton` | 清空 | 清空自动保存目录，回退默认目录 | 已实现 |
| `CaptureSettingsDialog.saveButton` | 保存设置 | 提交设置并关闭弹窗 | 已实现 |
| `CaptureSettingsDialog.cancelButton` | 取消 | 放弃修改并关闭弹窗 | 已实现 |
| `AboutDialog.copyInfoButton` | 复制信息 | 复制应用信息到剪贴板 | 已实现 |
| `AboutDialog.closeButton` | 关闭 | 关闭关于弹窗 | 已实现 |
| `CloseDialog.hideToTrayButton` | 隐藏到托盘 | 返回隐藏到托盘选择 | 已实现 |
| `CloseDialog.exitAppButton` | 退出应用 | 返回退出应用选择 | 已实现 |
| `CloseDialog.cancelButton` | 取消 | 取消关闭 | 已实现 |

## 6. 备注

- 长截图主路径：区域模式选区后通过 Overlay 工具条进入。
- 长截图当前仍按实验特性管理，需开启 `SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE` 才参与构建与运行。
- `btnHistory` / `btnMoreRecent` / `btnMoreFormat` 当前都已接线，但行为是简化版，不等于完整页面跳转。
- 文档状态以当前代码实现为准。