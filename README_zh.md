<p align="center">
  <img src="doc/images/logo.svg" width="400" alt="LimboAI 标志">
</p>

# LimboAI - Godot 4 的行为树与状态机插件

<!--
[![🔗 全部构建](https://github.com/limbonaut/limboai/actions/workflows/all_builds.yml/badge.svg)](https://github.com/limbonaut/limboai/actions/workflows/all_builds.yml)
-->
[![🔎 单元测试](https://github.com/limbonaut/limboai/actions/workflows/test_builds.yml/badge.svg)](https://github.com/limbonaut/limboai/actions/workflows/test_builds.yml)
[![文档状态](https://readthedocs.org/projects/limboai/badge/?version=latest)](https://limboai.readthedocs.io/zh-cn/latest/?badge=latest)
[![GitHub 许可证](https://img.shields.io/github/license/limbonaut/limboai)](https://github.com/limbonaut/limboai/blob/master/LICENSE.md)
[![Discord](https://img.shields.io/discord/1185664967379267774?logo=discord&link=https%3A%2F%2Fdiscord.gg%2FN5MGC95GpP)](https://discord.gg/N5MGC95GpP)
[![Mastodon 关注](https://img.shields.io/mastodon/follow/109346796150895359?domain=https%3A%2F%2Fmastodon.gamedev.place)](https://mastodon.gamedev.place/@limbo)

> **支持的 Godot 引擎版本：** **4.4 - 4.6**
> *（旧版本支持情况见下文）*

**LimboAI** 是一个开源的 C++ 插件，专为 **Godot Engine 4** 设计，提供了 **行为树** 与 **状态机** 的组合，二者可协同工作，创造出复杂的 AI 行为。它内置了行为树编辑器、内置文档、可视化调试器、包含教程的详尽演示项目等等！虽然插件采用 C++ 实现，但它完全支持使用 GDScript [创建自定义任务](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/custom-tasks.html) 和 [状态](https://limboai.readthedocs.io/zh-cn/stable/hierarchical-state-machines/create-hsm.html)。

如果您喜欢使用 LimboAI，请考虑在 Ko-fi 上**捐赠支持**我的开发工作 😊 您的贡献将帮助我持续改进和完善它。

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Y8Y2TCNH0)

![带纹理的截图](doc/images/behavior-tree-editor-debugger.png)

行为树是一种强大的层次化结构，用于建模和控制游戏中代理（如角色、敌人）的行为。它们旨在让您更轻松地为游戏创建丰富且高度模块化的行为。要了解更多关于行为树的信息，请查看 [行为树简介](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/introduction.html) 以及我们的演示项目（包含教程）。

## 演示

![演示中的 Charger](doc/images/demo_charger.gif)

> [!NOTE]
> **演示项目**位于 `demo` 文件夹中，也可在 [**Releases**](https://github.com/limbonaut/limboai/releases) 页面单独下载。
> 运行 `demo/scenes/showcase.tscn` 即可开始体验。
> 它还包含一个教程，通过示例介绍行为树的基本概念。

### 视频

> 由各位创作者制作的 YouTube 视频

<a href="https://www.youtube.com/watch?v=cGqO7SVKqkM"><img src="https://img.youtube.com/vi/cGqO7SVKqkM/0.jpg" width=272></a>
<a href="https://www.youtube.com/watch?v=E_FIy2dTkNc"><img src="https://img.youtube.com/vi/E_FIy2dTkNc/0.jpg" width=272></a>
<a href="https://www.youtube.com/watch?v=45DaBV9FgOQ"><img src="https://img.youtube.com/vi/45DaBV9FgOQ/0.jpg" width=272></a>
<a href="https://www.youtube.com/watch?v=vZHzMO90IwQ"><img src="https://img.youtube.com/vi/vZHzMO90IwQ/0.jpg" width=272></a>
<a href="https://www.youtube.com/watch?v=gAk3xl5fBsM"><img src="https://img.youtube.com/vi/gAk3xl5fBsM/0.jpg" width=272></a>
<a href="https://www.youtube.com/watch?v=aP0Aacdxmno"><img src="https://img.youtube.com/vi/aP0Aacdxmno/0.jpg" width=272></a>

## 支持的 Godot 版本

| 插件版本                | GDExtension/AssetLib   | 模块     |
|--------------------------|------------------------|----------|
| `1.6.x` 发行版           | Godot 4.4, 4.5, 4.6    | Godot 4.6|
| `1.5.x` 发行版           | Godot 4.4, 4.5         | Godot 4.5|
| `1.4.x` 发行版           | Godot 4.4, 4.5         | Godot 4.4|
| `1.2.0`-`1.3.x` 发行版   | Godot 4.3              | Godot 4.3|
| `1.1.x` 发行版           | Godot 4.2              | Godot 4.2|

## 功能特性

- **行为树（BT）：**
    - 在编辑器中轻松创建、编辑和保存 `BehaviorTree` 资源。
    - 使用 `BTPlayer` 节点执行 `BehaviorTree` 资源。
    - 通过组合和嵌套任务，构建复杂的层次化行为。
    - 使用组合、装饰器和条件任务控制执行流程。
    - [创建自定义任务](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/custom-tasks.html)：继承核心类 `BTAction`、`BTCondition`、`BTDecorator` 和 `BTComposite`。
    - 内置类文档。
    - 黑板系统：通过 `Blackboard` 在任务间无缝共享数据。
      - 黑板计划：在 BehaviorTree 资源中定义变量，并在 BTPlayer 节点中覆盖其值。
      - 计划编辑器：管理变量及其数据类型和属性提示。
      - 黑板作用域：防止名称冲突，并支持高级技巧，如[在多个代理间共享数据](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/using-blackboard.html#sharing-data-between-several-agents)。
      - 黑板参数：[导出 BB 参数](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/using-blackboard.html#task-parameters)，用户可为参数提供值或将其绑定到黑板变量（可用于自定义任务）。
      - 检查器支持指定黑板变量（为以 "_var" 结尾的导出 `StringName` 属性提供自定义编辑器）。
    - 使用 `BTSubtree` 任务执行来自不同资源文件的树，提升组织性和可复用性。
    - 可视化调试器：在运行场景中检查任意行为树的执行情况，以识别和解决问题。
    - 使用 `BehaviorTreeView` 节点在游戏内可视化行为树（用于自定义游戏内工具）。
    - 通过自定义性能监视器监控树性能。

- **分层状态机（HSM）：**
    - 继承 `LimboState` 类来实现状态逻辑。
    - `LimboHSM` 节点作为状态机，管理 `LimboState` 实例和状态转换。
    - `LimboHSM` 本身也是一个状态，可以嵌套在其他 `LimboHSM` 实例中。
    - [基于事件](https://limboai.readthedocs.io/zh-cn/stable/hierarchical-state-machines/create-hsm.html#events-and-transitions)：转换与事件关联，当相关事件被派发时由状态机触发，从而更好地解耦转换与状态逻辑。
    - 结合状态机与行为树：使用 `BTState` 实现高级响应式 AI。
    - 委托选项：使用原生 `LimboState`，将实现[委托给回调函数](https://limboai.readthedocs.io/zh-cn/stable/hierarchical-state-machines/create-hsm.html#single-file-state-machine-setup)，非常适合快速原型开发和游戏开发挑战赛。
    - 注意：状态机的设置和初始化需要编写代码；目前没有图形界面编辑器。

- **测试覆盖：** 行为树任务和 HSM 均有单元测试覆盖。

- **GDExtension：** LimboAI 可以作为 [扩展使用](https://limboai.readthedocs.io/zh-cn/stable/getting-started/getting-limboai.html#get-gdextension-version)。无需自定义引擎构建。

- **演示 + 教程：** 查看我们详尽的演示项目，其中包含通过示例介绍行为树的入门教程。

## 第一步

按照 [入门指南](https://limboai.readthedocs.io/zh-cn/stable/getting-started/getting-limboai.html) 学习如何开始使用 LimboAI 和演示项目。

## 获取 LimboAI

LimboAI 可以作为 C++ 模块或 GDExtension 共享库使用。GDExtension 版本使用更方便，但功能上略有局限。无论您选择哪种方式，项目都能保持兼容，并且您可以随时切换。参见 [使用 GDExtension](https://limboai.readthedocs.io/zh-cn/stable/getting-started/getting-limboai.html#get-gdextension-version)。

### 预编译版本

- 对于最新构建版本，请进入 **Actions** → [**全部构建**](https://github.com/limbonaut/limboai/actions/workflows/all_builds.yml)，从列表中选择一个构建，向下滚动直到找到 **Artifacts** 部分。
- 对于发行版构建，请查看 [**Releases**](https://github.com/limbonaut/limboai/releases)。

### 从源码编译

- 下载 Godot 引擎源代码，并将本模块源代码放入 `modules/limboai` 目录。
- 关于 [如何从源代码构建](https://docs.godotengine.org/zh-cn/stable/engine_details/development/compiling/index.html)，请参考 Godot 引擎文档。
- 如果您计划导出使用了 LimboAI 模块的游戏，还需要构建导出模板。
- 要执行单元测试，请使用 `tests=yes` 编译引擎，并以 `--test --tc="*[LimboAI]*"` 参数运行。

#### 对于 GDExtension

- 您需要 SCons 构建工具和 C++ 编译器。另请参阅 [编译](https://docs.godotengine.org/zh-cn/stable/contributing/development/compiling/index.html)。
- 运行 `scons target=editor` 为当前平台构建插件库。
  - 如果 `limboai/godot-cpp` 目录中尚不存在 godot-cpp 仓库，SCons 会自动克隆。
  - 默认情况下，构建的目标文件会放置在演示项目中：`demo/addons/limboai/bin/`
- 查看 `scons -h` 获取其他选项和目标。

## 使用插件

- 在线文档：[稳定版](https://limboai.readthedocs.io/zh-cn/stable/index.html)，[最新版](https://limboai.readthedocs.io/zh-cn/latest/index.html)
- [入门指南](https://limboai.readthedocs.io/zh-cn/stable/getting-started/getting-limboai.html)
- [行为树简介](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/introduction.html)
- [在 GDScript 中创建自定义任务](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/custom-tasks.html)
- [使用黑板共享数据](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/using-blackboard.html)
- [访问场景树中的节点](https://limboai.readthedocs.io/zh-cn/stable/behavior-trees/accessing-nodes.html)
- [状态机](https://limboai.readthedocs.io/zh-cn/stable/hierarchical-state-machines/create-hsm.html)
- [使用 GDExtension](https://limboai.readthedocs.io/zh-cn/stable/getting-started/getting-limboai.html#get-gdextension-version)
- [在 C# 中使用 LimboAI](https://limboai.readthedocs.io/zh-cn/stable/getting-started/c-sharp.html)
- [类参考](https://limboai.readthedocs.io/zh-cn/stable/classes/featured-classes.html)

## 贡献

欢迎贡献！对于 Bug 报告、功能请求或代码更改，请开启 Issue。关于贡献代码或文档的详细指南，请查看我们的 [贡献指南](https://limboai.readthedocs.io/zh-cn/latest/getting-started/contributing.html) 页面。

如果您有关于行为树任务或功能的想法，且能在多种项目中发挥作用，请开启 Issue 进行讨论。

## 社区交流

需要帮助？我们有 Discord 服务器：https://discord.gg/N5MGC95GpP

我在 Mastodon 上撰写关于 LimboAI 开发的内容：https://mastodon.gamedev.place/@limbo

## 许可证

本源代码的使用受 MIT 式许可证管理，可在 LICENSE 文件中找到，或访问 https://opensource.org/licenses/MIT

LimboAI 标志和演示项目的艺术资产采用知识共享署名 4.0 国际许可证，详情请见 https://creativecommons.org/licenses/by/4.0/
