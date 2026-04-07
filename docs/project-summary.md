# Project Summary

本文档基于当前仓库代码整理，目标是快速说明这个项目现在已经具备什么能力，以及主程序和基础设施分别发展到了什么阶段。

## 1. 项目定位

当前工程是一个基于 Qt 6 + Qt3D + CMake 的 3D 编辑器原型。

它同时包含两层内容：

- 一个已经可以运行和交互的桌面 demo，围绕 primitive / polygon / extrude 对象编辑展开
- 一组可复用的底层模块，包括可观察对象系统、事件总线、Qt3D 渲染节点体系，以及几何和曲线扩展

从仓库当前状态来看，这个项目已经超过了“Qt3D 示例程序”的范畴，更接近一个可继续扩展的小型 3D 建模/编辑器底座。

## 2. 主程序当前已经实现的功能

### 2.1 主窗口和界面布局

主程序由 `src/main.cpp` 和 `src/mainwindow.*` 启动，当前界面已经包含：

- 顶部菜单栏，包含 `File -> Exit`
- 顶部对象创建工具区
- 左侧对象层级树
- 中央 Qt3D 视图和视角工具栏
- 右侧可滚动属性面板
- 底部状态栏，默认显示 `Ready`

整体布局使用 `QSplitter` 组织三栏结构，左右面板都支持拖拽调整宽度。

### 2.2 对象创建

当前主程序支持创建 8 类对象：

- Box
- Sphere
- Cylinder
- Cone
- Line
- Ring
- Polygon
- Extrude

点击顶部工具按钮后，系统会：

- 创建对应的 `SceneObject`
- 为对象分配递增名称，例如 `Box 1`、`Polygon 1`、`Extrude 1`
- 为对象分配默认出生位置，避免完全重叠
- 在 `SceneModel` 中创建对应的渲染节点
- 自动切换当前选中对象

对应入口主要在：

- `src/mainwindow.cpp`
- `src/scene/scene_controller.*`
- `src/scene/primitive_object.*`
- `src/scene/polygon_object.*`
- `src/scene/extrude_object.*`

### 2.3 对象层级树

左侧对象树由 `src/hierarchy.*` 实现，当前支持：

- 展示场景对象的树形层级
- 根据父子关系嵌套显示对象
- 选中树节点切换当前对象
- 自动展开整个对象树
- 在树中直接重命名对象

树节点重命名会直接同步回对象数据模型，并联动属性面板和 3D 渲染层。

### 2.4 属性面板

右侧属性面板由 `src/property_panel.*` 及其 `sections/`、`editors/` 子模块组成，当前支持：

- 对象通用属性
  - Name
  - Type（只读）
  - Parent
  - Color
  - Delete Object
- 变换属性
  - Position（X / Y / Z）
  - Rotation（X / Y / Z）
- primitive 专属尺寸属性
- polygon 专属编辑区
- extrude 专属编辑区

对象区中的 `Parent` 支持将对象挂到场景根节点或其他对象下，并会过滤掉非法父节点选择。

`Delete Object` 带确认对话框，避免误删。

### 2.5 Primitive 属性编辑

当前基础图元支持的参数编辑包括：

- Box: Width / Height / Depth
- Sphere: Radius
- Cylinder: Radius / Height
- Cone: Radius / Height
- Line: Start / End 点坐标
- Ring: Inner Radius / Outer Radius / Length / Start Angle / End Angle

这些属性变化会实时同步到对应的 Qt3D 渲染节点。

### 2.6 Polygon 编辑能力

`PolygonObject` 已接入独立属性区，当前支持：

- 顶点列表编辑
- 新增顶点
- 顶点重排
- 删除顶点
- 拉伸高度 `Height`
- 分层数 `Rings`

当顶点数不足 3 时，属性面板会显示校验提示，说明 polygon 无法正常渲染。

### 2.7 Extrude 编辑能力

`ExtrudeObject` 已接入独立属性区，当前支持：

- 路径类型切换
  - Line
  - Cubic Bezier
  - Ellipse
  - Arc
- 路径控制点编辑
- 路径分段数 `Divisions`
- 椭圆路径旋转角 `Rotation`
- 起止角 `Start Angle / End Angle`
- 截面轮廓顶点编辑
- 轮廓顶点新增、重排和删除

面板会分别对路径合法性和 profile 合法性给出提示，因此这一块已经不是静态演示，而是带基本约束和数据校验的编辑器能力。

### 2.8 父子层级与重挂接

`SceneController::setObjectParent` 当前已经支持：

- 禁止对象挂到自己身上
- 禁止形成循环父子关系
- 重挂接时保持对象世界坐标不变
- 重挂接时保持对象世界旋转不变
- 重挂接后刷新层级树和渲染层父子关系

这意味着项目已经具备基础的 scene graph 组织能力，而不是简单的平铺对象列表。

### 2.9 3D 视图与交互

中央区域通过 `Graph3dScene` 嵌入 `Qt3DWindow`，当前已接入：

- Qt3D 场景根节点
- 自定义 frame graph / renderer
- 背景色设置
- 双方向光照
- 右下角坐标轴 gizmo
- 场景对象渲染节点同步
- 对象选中高亮
- `QObjectPicker` 驱动的对象点击选中

主窗口事件过滤器还处理了“点击空白区域取消选中”的交互逻辑，因此当前 3D 视图已经具备基本编辑器手感。

### 2.10 相机能力

`src/q3dextension/extras/camera_controller.*` 已接入主界面，当前支持：

- `Reset`
- `XY`
- `XZ`
- `YZ`
- `Fit`

并支持常见鼠标交互：

- 左键拖动旋转
- 右键拖动平移
- 滚轮缩放

### 2.11 资源与界面细节

当前工程已经接入：

- 工具栏 SVG 图标资源
- 主窗口样式表
- Qt Linguist 翻译入口

截图中可以看到，这套 UI 已经形成了比较完整的“左树 + 中视图 + 右属性”编辑器原型界面。

### 2.12 Undo / Redo

当前主程序已经接入基于 `QUndoStack` 的撤销 / 重做能力，并通过主菜单 `Edit` 暴露出来，默认支持常见快捷键：

- `Ctrl+Z`
- `Ctrl+Y`

当前会进入历史记录的操作包括：

- 创建对象
- 删除对象
- 对象重命名
- 颜色修改
- 位移和旋转修改
- 父子重挂接
- primitive 参数修改
- line type 修改
- polygon / extrude 的参数与顶点编辑

当前没有纳入历史的内容包括：

- 当前选中对象切换
- 相机视角与视图操作

## 3. 数据流和同步机制

### 3.1 数据模型

场景对象统一基于 `SceneObject` / `PrimitiveObject` / `PolygonObject` / `ExtrudeObject` 组织。

对象基础属性包括：

- name
- position
- rotation
- color
- parent-child relationship

不同对象类型再扩展各自的尺寸、顶点、路径和 profile 数据。

### 3.2 变更传播

当前项目已经形成一条较清晰的数据同步链路：

- 对象属性修改触发 `QObservableObject::propertyChanged`
- `SceneController` 将变化上抛为 `objectChanged`
- `Hierarchy` 和 `PropertyPanel` 刷新当前 UI
- `SceneModel` / RenderNode 同步刷新 Qt3D 实体

因此主程序已经是“数据驱动 + 视图实时同步”的工作方式，而不是手工刷新每个控件。

### 3.3 选中态联动

当前选中状态可以从多个入口驱动：

- 左侧层级树
- 顶部创建动作后的自动选中
- 中央 3D 视图点击拾取
- 空白点击取消选中

`SceneController` 统一管理当前对象，`SceneModel` 再负责渲染层的 select / deselect。

## 4. 仓库中的基础设施能力

这一部分不一定全部在当前 demo UI 中直接暴露，但代码已经存在，并且已经和主程序形成连接。

### 4.1 可观察对象系统

`src/core/observable_object.*`、`src/core/x_property.*`、`src/core/x_property_validator.*` 提供了一套自定义属性系统，具备：

- 属性变化通知
- 嵌套变化传播
- validator 支持
- grouped change 行为
- 对象级别的通知控制

这部分已经由 `tests/core` 中的 doctest 用例覆盖核心行为。

### 4.2 事件总线

`src/core/event_bus.*` 提供轻量事件总线能力，支持：

- 发送事件
- 异步投递事件
- watch / unwatch 事件类型
- 泛型事件封装

目前这部分主要作为工程基础设施存在，并在核心测试中得到覆盖。

### 4.3 Qt3D 渲染节点体系

`src/visualization/graph3d` 目录下已经形成了较清晰的渲染抽象，包括：

- `SceneModel`
- `RenderNodeFactory`
- `RenderNode`
- 渲染节点实体创建与更新
- 渲染节点父子同步
- 选中态同步

当前主程序已经把这套机制接到了：

- Box / Sphere / Cylinder / Cone / Line / Ring
- Polygon
- Extrude

### 4.4 曲线、几何与 shape 扩展

`src/q3dextension` 下已经沉淀出一批底层几何能力，例如：

- 曲线
  - `LineCurve`
  - `ArcCurve`
  - `EllipseCurve`
  - `BezierCurve`
- 几何与网格
  - `LineGeometry` / `LineMesh`
  - `RingGeometry` / `RingMesh`
  - `PolygonGeometry` / `PolygonMesh`
  - `ExtrudeShapeGeometry` / `ExtrudeShapeMesh`
  - `EdgeMesh`
- 轮廓 / shape
  - `Shape`
  - `PolygonProfileShape`
  - `TrapezoidShape`

这说明仓库已经为后续更复杂的对象类型预留了良好的基础。

### 4.5 工具与辅助模块

仓库里还保留了多类辅助模块：

- `src/utils/logging.*`
- `src/utils/crash_handler.*`
- `src/utils/color_util.*`
- `src/utils/compare_util.*`
- `src/utils/collection_util.*`
- `src/core/profiler.*`
- `bin/breakpad/*`

这些内容表明项目在调试、崩溃处理和辅助工具链方面也已经做了一定准备。

## 5. 构建与测试现状

### 5.1 构建系统

工程当前具备：

- CMake 主工程
- CMake Presets
- 可选 vcpkg 集成
- 可选 Breakpad 集成
- Qt Linguist 翻译生成
- 可选测试构建开关

关键开关包括：

- `QT3D_DEMO_USE_VCPKG`
- `QT3D_DEMO_ENABLE_BREAKPAD`
- `QT3D_DEMO_BUILD_TESTS`

## 6. 当前边界和未见实现内容

- 场景保存 / 加载
- 文件导入 / 导出
- 撤销 / 重做
- 多选与框选
- 复制 / 粘贴对象
- 时间轴 / 动画编辑
- 更完整的业务对象体系接入
- GUI 自动化测试
