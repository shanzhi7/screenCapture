# 按钮映射（同步版）

> 更新时间：2026-03-14

## 1. MainWindow 主界面按钮

| objectName | 按钮文案/入口 | 实际行为 | 状态 |
| --- | --- | --- | --- |
| `modeFullButton` | 全屏 | 切换到全屏截图模式 | 已实现 |
| `modeRegionButton` | 区域 | 切换到区域截图模式 | 已实现 |
| `modeScrollButton` | 滚动 | 独立滚动模式入口，当前界面隐藏 | 已隐藏 |
| `startCaptureButton` | 开始截图 | 按当前模式启动截图 | 已实现 |
| `sideCaptureButton` | 侧栏拍摄 | 按当前模式启动截图，行为等价 `startCaptureButton` | 已实现 |
| `sideGalleryButton` | 侧栏图库 | 打开当前生效的自动保存目录 | 已实现 |
| `btnPrev` | 上一张 | 在历史浏览上下文切到更早一张截图 | 已实现 |
| `btnShotIcon` | 中间截图图标 | 回到最新截图并刷新主预览区 | 已实现 |
| `btnNext` | 下一张 | 在历史浏览上下文切到更新一张截图 | 已实现 |
| `btnTopSettings` | 顶部设置 | 打开截图设置弹窗 | 已实现 |
| `sideConfigButton` | 侧栏设置 | 打开截图设置弹窗 | 已实现 |
| `btnHotkeySetting` | 热键设置 | 打开截图设置弹窗 | 已实现 |
| `sideBottomButton` | 关于 | 打开关于弹窗 | 已实现 |
| `btnAutoSave` | 自动保存 | 切换自动保存开关并持久化 | 已实现 |
| `btnFormatSetting` | 格式 PNG/JPG | 切换输出格式 | 已实现 |
| `btnMoreFormat` | 更多格式 | 当前直接切换输出格式 | 已实现（简化行为） |
| `btnMoreRecent` | 更多最近截图 / 返回最近 | 最近页进入图库页；图库页返回最近页 | 已实现 |

## 2. 历史浏览区域

| objectName | 按钮文案/入口 | 实际行为 | 状态 |
| --- | --- | --- | --- |
| `recentThumbButton` | 历史缩略图 | `recentPage` 单击加载主预览；拖出主窗口创建贴图。`galleryPage` 保持单击加载主预览 | 已实现 |
| `historyStackedWidget` | 最近页 / 图库页容器 | 在 `recentPage` 与 `galleryPage` 间切换 | 已实现 |
| `galleryPage` | 图库页 | 展示完整历史网格，复用主预览区 | 已实现 |

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
| 贴图 | 普通区域截图与长截图都可创建置顶悬浮贴图窗，同时复制到剪贴板并写入历史 | 已实现 |
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
| `CaptureSettingsDialog.launchAtStartupCheckBox` | 开机自启动 | 切换当前用户登录后的应用自启动状态，默认关闭 | 已实现 |
| `CaptureSettingsDialog.saveButton` | 保存设置 | 提交设置并关闭弹窗 | 已实现 |
| `CaptureSettingsDialog.cancelButton` | 取消 | 放弃修改并关闭弹窗 | 已实现 |
| `AboutDialog.copyInfoButton` | 复制信息 | 复制应用信息到剪贴板 | 已实现 |
| `AboutDialog.closeButton` | 关闭 | 关闭关于弹窗 | 已实现 |
| `CloseDialog.hideToTrayButton` | 隐藏到托盘 | 返回隐藏到托盘选择 | 已实现 |
| `CloseDialog.exitAppButton` | 退出应用 | 返回退出应用选择 | 已实现 |
| `CloseDialog.cancelButton` | 取消 | 取消关闭 | 已实现 |


## 6. PinnedImageWindow 贴图窗

| 按钮 / 交互 | 实际行为 | 状态 |
| --- | --- | --- |
| 最小化 | 进入系统任务栏分组预览，可从主程序任务栏预览恢复任意贴图 | 已实现 |
| 最大化 / 还原 | 放大到当前屏幕工作区内最大可视尺寸，再次点击恢复原尺寸与位置 | 已实现 |
| 关闭 | 关闭当前贴图窗 | 已实现 |
| 拖动窗口 | 非最大化状态下可拖动位置 | 已实现 |
| 滚轮缩放 | 非最大化状态下按比例缩放贴图 | 已实现 |
## 7. 备注

- `sideGalleryButton` 当前用于打开自动保存目录；图库入口仅保留左侧历史区内的 `btnMoreRecent`。
- 主界面左侧历史区当前分为 `recentPage` 与 `galleryPage` 两页；右侧 `formatPreviewLabel` 继续作为统一预览区。
- `recentPage` 的历史缩略图支持拖出主窗口创建贴图；`galleryPage` 当前不支持拖拽贴图。
- 图库页第一版仅做完整历史浏览，不含删除、筛选、分页、搜索。
- 长截图主路径：区域模式选区后通过 Overlay 工具条进入。
- 长截图当前仍按实验特性管理；当前仓库默认构建开启，如关闭 `SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE` 则不参与构建与运行。
- 文档状态以当前代码实现为准。
