# Codex 代码协作规则

> 更新时间：2026-03-07

1. 禁止直接提交到主分支。
2. 仅修改与当前任务相关的文件。
3. 不进行与任务无关的重构。
4. 每个任务保持单一目标，避免混入无关改动。
5. 遵循项目现有目录结构与命名规则。
6. 默认使用简体中文沟通与文档。
7. 源码与文档统一使用 UTF-8 编码，避免乱码。
8. 文档变更需与代码实现状态保持一致。
9. R-ARCH-001：为保证可维护性与高内聚低耦合，新增功能优先通过新建类文件拆分模块，不向 `MainWindow` 持续堆叠多职责逻辑。
10. R-CONFIG-001：配置访问统一通过 `SettingsService`，禁止在业务层硬编码 `QSettings(organization, application)` 命名空间。
11. R-UI-001：设置入口职责固定，`btnTopSettings` / `sideConfigButton` / `btnHotkeySetting` 统一打开 `CaptureSettingsDialog`，`sideBottomButton` 统一打开 `AboutDialog`。
12. R-OVERLAY-001：截图覆盖层必须吞掉右键与菜单相关事件，并提供“右键取消 = ESC = ✕”的一致交互语义。
