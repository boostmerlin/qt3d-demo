# Project Summary

本文档基于当前工程代码整理，目标是快速说明这个项目“已经具备什么能力”。

## 1. 项目定位

当前仓库是一个基于 Qt 6 + Qt3D + CMake 的 3D 编辑器原型工程。

从实际代码看，它包含两层内容：

- 一层是已经能运行的 demo 应用：一个可交互的 primitive 场景编辑器
- 一层是可复用的基础设施：可观察对象系统、事件总线、Qt3D 渲染节点体系、几何/曲线/网格扩展

## 2. 当前主程序已经实现的功能

### 2.1 主窗口与整体布局

主程序由 `src/main.cpp` 和 `src/mainwindow.*` 启动，当前界面已经具备：

- 顶部菜单栏，包含 `File -> Exit`
- 顶部 primitive 工具区
- 左侧对象层级树
- 中间 Qt3D 视图
- 右侧属性面板
- 底部状态栏，默认显示 `Ready`

主界面使用 `QSplitter` 组织三栏布局，左右面板都支持拖拽调整宽度。

### 2.2 Primitive 创建

当前 demo 已支持创建 4 种基础几何体：

- Box
- Sphere
- Cylinder
- Cone

点击顶部按钮后，系统会自动：

- 创建对应的 `PrimitiveObject`
- 分配递增名称，如 `Box 1`、`Sphere 2`
- 分配一个默认出生位置，避免多个对象完全重叠
- 为对象创建对应的 Qt3D 渲染节点
- 自动切换当前选中对象

相关实现位于：

- `src/demo/primitive_object.*`
- `src/demo/scene_controller.*`
- `src/demo/primitive_render_node.*`

### 2.3 对象层级树

左侧对象树由 `src/hierarchy.*` 实现，当前已支持：

- 展示场景对象的树形结构
- 根据 parent-child 关系嵌套显示对象
- 点击树节点切换当前选中对象
- 展开整个对象树
- 直接在树中重命名对象

树节点重命名后会同步回对象数据模型，并联动右侧属性面板与 3D 视图。

### 2.4 属性面板

右侧属性面板由 `src/property_panel.*` 实现，当前已经支持编辑以下属性：

- Name
- Type（只读）
- Parent
- Color
- Position（X/Y/Z）
- Rotation（X/Y/Z）
- Dimensions（按不同 primitive 动态显示）
- Delete Object

其中：

- `Parent` 支持把对象挂到场景根节点或其他对象下
- `Color` 会弹出颜色选择器，并实时更新 3D 材质颜色
- `Dimensions` 会根据对象类型动态变化
  - Box: Width / Height / Depth
  - Sphere: Radius
  - Cylinder: Radius / Height
  - Cone: Radius / Height
- `Delete Object` 支持删除当前对象

当前属性面板已经具备基本的编辑状态切换：

- 未选中对象时显示空状态
- 选中对象时自动刷新表单
- 编辑对象后会立即同步到数据层和渲染层

### 2.5 父子层级与重挂接

当前工程已经支持对象父子关系，而不仅仅是平铺对象列表。

`SceneController::setObjectParent` 已实现：

- 防止把对象挂到自己身上
- 防止循环父子关系
- 重挂接时保持对象世界坐标和世界旋转不变
- 重挂接后同步更新层级树和 3D 场景中的父子实体关系

这意味着当前 demo 已支持基本的层级组织能力。

### 2.6 3D 视图与场景渲染

中间区域通过 `Graph3dScene` 嵌入 `Qt3DWindow`，当前已具备：

- Qt3D 主场景根节点
- 自定义 frame graph / renderer
- 双方向光照
- 右下角坐标轴 gizmo
- 相机控制
- 对象创建、更新、删除
- 选中对象高亮显示

当前 demo 的 4 类 primitive 都有自己的渲染节点：

- `BoxPrimitiveRenderNode`
- `SpherePrimitiveRenderNode`
- `CylinderPrimitiveRenderNode`
- `ConePrimitiveRenderNode`

这些节点已经会根据对象属性同步：

- 名称
- 局部位移
- 局部旋转
- 尺寸
- 颜色
- 选中高亮状态

### 2.7 相机操作

`src/q3dextension/extras/camera_controller.*` 中已经实现较完整的相机控制逻辑，当前主界面已接入：

- Reset
- XY
- XZ
- YZ
- Fit

同时支持鼠标交互：

- 左键拖动旋转
- 右键拖动平移
- 滚轮缩放
- 双键/组合轴输入缩放

还支持平移/旋转方向反转配置。

### 2.8 资源与界面细节

工程当前还包含一些已接入资源：

- 4 个 primitive toolbar 图标
- Qt Linguist 翻译入口
- 主窗口样式表

翻译基础设施已接入，但仓库中的 `qt3d-demo_zh_CN.ts` 目前仍比较初步。

## 3. 当前 demo 的数据与同步机制

### 3.1 Primitive 数据模型

`PrimitiveObject` 继承自 `QObservableObject`，当前基础属性包括：

- name
- position
- rotation
- color

不同子类再补充自己的尺寸属性。

### 3.2 变更自动传播

SceneController 与 SceneModel 已建立对象变化联动机制：

- 对象属性变化会发出 `propertyChanged`
- SceneController 把变化转成 `objectChanged`
- PropertyPanel / Hierarchy 接收后刷新 UI
- SceneModel 接收后刷新 RenderNode

因此当前对象编辑属于“数据驱动 + 实时渲染同步”模式。

### 3.3 选中态联动

当前已经实现：

- 层级树驱动当前对象切换
- SceneController 管理当前选中对象
- SceneModel 对应 RenderNode 执行 select / deselect
- 3D 材质在选中时显示更亮颜色

## 4. 工程里已经具备的基础设施能力

这一部分不一定全部在当前 demo UI 中直接暴露，但代码已经存在，可复用价值很高。

### 4.1 可观察对象系统

`src/core/observable_object.*` + `src/core/x_property.*` + `src/core/x_property_validator.*` 提供了一套自定义属性系统：

- 基于 `QProperty` 的 `X_PROPERTY` 宏
- 自动绑定 Qt Property / Bindable
- 属性变化通知
- 嵌套对象变化传播
- 父子 observable 链接
- shot 机制控制通知次数
- guard 方式临时禁用/恢复通知
- group change 机制把多次变更合并成一次通知
- 属性校验器
- 属性 deleter

这套机制已经通过测试覆盖了大量场景，包括：

- 基本属性读写
- 嵌套对象传播
- map/hash/list/pair 等容器
- validator 逻辑
- deleter 逻辑
- grouped change 行为

### 4.2 事件总线

`src/core/event_bus.*` 提供了一套轻量事件总线系统，包含：

- 全局发送事件
- 异步投递事件
- watch / unwatch 指定事件类型
- `WithEventNotifier` / `EventNotifierT` 模板辅助类
- 自动移除型事件过滤器 `SelfRemoveEventFilter`
- 可携带泛型数据的 `GenericEvent`

测试中已经覆盖：

- QApplication 环境下事件转发
- send / post 两种模式
- 不同继承结构下 notifier 的工作方式
- 自移除 event filter 行为

### 4.3 Qt3D 渲染节点体系

`src/visualization/graph3d` 目录下已经形成一套比较完整的渲染抽象：

- `SceneModel`
- `RenderNodeFactory`
- `RenderNode`
- `RenderNodeEntity`
- `ProxyRenderNode`
- `Selectable`
- `WithColor`
- `WithMaterial`
- `WithAlpha`

当前这套体系已经支持：

- 按对象类型注册渲染节点创建器
- 为数据对象生成对应 Qt3D 实体
- 对属性变化做增量更新
- 节点删除清理
- 渲染节点父子同步
- 可选中对象的视觉反馈
- 材质切换与附加

### 4.4 Qt3D 组件与渲染辅助能力

现有 3D 扩展层还包括：

- `CustomRenderer`
  - 自定义 clear color
  - 主视口与 gizmo 视口
  - gizmo anchor 定位
  - debug overlay 开关
- `LayerManager`
  - 默认层、opaque、transparent 层管理
  - layer 附加与拆卸
- `GizmoAxis`
  - 三轴 gizmo
  - 轴颜色、镜像轴、轴名称、缩放等选项
- `Text3dEntity`
  - 3D 文本实体
- `BehaviorComponent`
  - 面向 RenderNode 的组件行为基类
- `ColourNodeComponent`
  - 材质颜色效果处理
- `DrawOutlineComponent`
  - 绘制轮廓

### 4.5 通用几何体 / 曲线 / 形状扩展

`src/q3dextension` 目录已经积累了较多底层几何工具：

- 曲线
  - `Curve`
  - `LineCurve`
  - `EllipseCurve`
  - `ArcCurve`
  - `BezierCurve`
- 2D/轮廓形状
  - `Shape`
  - `TrapezoidShape`
- 计算几何 / Qt3D 几何封装
  - `LineGeometry` / `LineMesh`
  - `PolygonGeometry` / `PolygonMesh`
  - `RingGeometry` / `RingMesh`
  - `ExtrudeShapeGeometry` / `ExtrudeShapeMesh`
  - `EdgeMesh`

这部分说明工程并不只是当前 primitive demo，而是已经具备继续扩展更复杂几何对象的基础。

### 4.6 预留的更多 RenderNode 类型

在 `src/visualization/graph3d/nodes` 下，除了 demo primitive 节点外，还已经实现/保留了通用渲染节点，例如：

- `BoxRenderNode`
- `CylinderRenderNode`
- `PolygonRenderNode`
- `RingRenderNode`
- `LineRenderNode`
- `TrapezoidRenderNode`
- `EdgeRenderNode`
- `BoxEdgeRenderNode`
- `SourcePlaneWaveRenderNode`
- `StructureRenderNode`
- `FdtdRegionRenderNode`
- `PortRenderNode`

这些节点说明仓库中保留了更大业务域模型的渲染骨架，只是当前主程序没有全部接回。

## 5. 工具与辅助能力

工程里还包含一些通用辅助模块：

- `src/utils/logging.*`
  - 控制台 / 文件日志处理器
- `src/utils/crash_handler.*`
  - crash handler 封装
- `src/utils/color_util.*`
  - 颜色运算辅助
- `src/utils/compare_util.*`
  - 浮点比较辅助
- `src/utils/collection_util.*`
  - 集合工具
- `src/core/profiler.*`
  - profiler 相关代码

此外仓库还保留了 breakpad 辅助脚本和说明，说明崩溃符号/堆栈分析链路已有一定准备。

## 6. 构建与测试现状

### 6.1 构建系统

工程已经具备：

- CMake 主工程
- CMake Presets
- 可选 vcpkg 集成
- 可选 breakpad 集成
- Qt Linguist 翻译生成
- 可选测试构建开关

关键开关：

- `QT3D_DEMO_USE_VCPKG`
- `QT3D_DEMO_ENABLE_BREAKPAD`
- `QT3D_DEMO_BUILD_TESTS`

### 6.2 测试

当前 `tests` 目录已经有 doctest 测试工程，主要覆盖核心基础层：

- `observable_test.cpp`
- `event_bus_test.cpp`

也就是说，当前测试主要针对“框架能力”，不是 GUI 自动化测试。

## 7. 当前边界与未见实现的内容

从当前代码看，下面这些能力尚未在主程序中看到完整实现：

- 场景保存 / 加载
- 文件导入 / 导出
- 撤销 / 重做
- 3D 视图直接拾取对象
- 多选与框选
- 复制 / 粘贴对象
- 动画时间轴或关键帧编辑
- 面向最终业务模型的完整编辑器接入
- GUI 自动化测试

换句话说，当前仓库已经有较扎实的 3D 编辑器骨架和基础设施，但主程序仍主要是一个 primitive 编辑 demo。

## 8. 一句话总结

当前工程已经不是一个“空壳 Qt3D 工程”，而是一个包含以下内容的可运行原型：

- 一个可交互的 primitive 场景编辑器
- 一套可观察对象与属性系统
- 一套可复用的事件总线
- 一套较完整的 Qt3D 渲染节点与组件体系
- 一批通用几何、曲线、图元扩展
- 基础测试、构建与资源组织能力

如果后续继续开发，这个仓库已经具备作为小型 3D 编辑器或领域建模工具原型底座的条件。
