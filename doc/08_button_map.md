# 按钮映射（同步版）

> 更新时间：2026-03-06

## 1. MainWindow 按钮状态

| objectName | 功能 | 状态 |
| --- | --- | --- |
| `modeFullButton` | 全屏截图模式 | 已实现 |
| `modeRegionButton` | 区域截图模式 | 已实现 |
| `modeScrollButton` | 独立滚动模式 | 不作为主路径（隐藏） |
| `startCaptureButton` | 启动截图 | 已实现 |
| `btnHotkeySetting` | 快捷触发截图 | 已实现（配置待实现） |
| `btnAutoSave` | 自动保存开关 | 仅 UI 切换 |
| `btnFormatSetting` | 输出格式设置 | 待实现 |
| `btnHistory` | 历史入口 | 待实现 |
| `btnMoreRecent` | 最近截图更多 | 待实现 |
| `btnMoreFormat` | 格式更多 | 待实现 |
| 其他导航按钮 | 业务跳转 | 占位提示 |

## 2. SelectionOverlay 工具条状态

| 按钮 | 功能 | 状态 |
| --- | --- | --- |
| 画笔/矩形/圆形/马赛克/文字 | 编辑工具 | UI 入口已就绪，逻辑待实现 |
| 长截图 | 进入长截图状态 | 已实现 |
| 保存 | 保存截图 | 已实现 |
| 取消 | 取消截图 | 已实现 |
| 确认（勾） | 普通截图复制 / 长截图复制 | 已实现 |

## 3. 备注

- 长截图主路径：区域模式选区后通过工具条进入。
- 文档状态以当前代码实现为准，不再使用旧的“独立滚动模式”描述。
