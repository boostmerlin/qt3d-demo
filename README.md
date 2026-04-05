# qt3d-demo

A 3D demo application built with Qt and CMake.

## 依赖

| 依赖 | 版本要求 |
|------|---------|
| Qt   | **6.x**（推荐 6.7+） |
| CMake | 3.21+ |
| vcpkg | 自动引导，或设置 `VCPKG_ROOT` 环境变量 |
| MSVC | 2019 / 2022 x64（Windows） |


## 构建

### 1. 准备本机 preset

复制模板并填写你的 Qt6 安装路径：

```powershell
Copy-Item CMakeUserPresets.json.eg CMakeUserPresets.json
```

编辑 `CMakeUserPresets.json`，将路径改为你本机的 Qt6 安装前缀，例如：

```json
"CMAKE_PREFIX_PATH": "D:/Qt/6.7.3/msvc2019_64",
"Qt6_DIR": "D:/Qt/6.7.3/msvc2019_64/lib/cmake/Qt6"
```

### 2. 配置 & 构建

```powershell
# Debug
cmake --preset local-debug
cmake --build --preset local-build-debug

# Release
cmake --preset local-release
cmake --build --preset local-build-release
```

可执行文件输出到 `build/presets/debug/Debug/qt3d-demo.exe`。

### 3. 在 JetBrains CLion 中使用

打开 **Settings → Build, Execution, Deployment → CMake**，新建 Profile 并在
**CMake preset** 下拉中选择 `local-debug`。CLion 会自动读取
`CMakeUserPresets.json` 中的路径和环境变量，无需手动填写。

## Preset 说明

| Preset | 说明 |
|--------|------|
| `debug` / `release` | 共享基础 preset（入仓库），不含本机路径 |
| `local-debug` / `local-release` | 本机 preset（`CMakeUserPresets.json`，不入仓库），含 Qt6 绝对路径 |
| `local-build-debug` / `local-build-release` | 对应的构建 preset，只构建主目标 `qt3d-demo` |

