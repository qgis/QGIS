# QGIS Windows构建工具说明

此目录包含用于在Windows平台上构建QGIS的各种脚本和配置文件。主要提供三种不同的构建方法：MinGW交叉编译、Cygwin原生编译和开发环境配置。

## 目录结构

```
ms-windows/
├── mingw/          # MinGW交叉编译相关文件
│   ├── build.sh    # 主构建脚本
│   └── mingwdeps.sh # 依赖安装脚本
├── cygwin/         # Cygwin构建相关文件
│   └── package.sh  # Cygwin包构建脚本
└── dev/            # 开发环境配置
    └── qgis.env.in # 环境变量模板文件
```

## 文件详细说明

### 1. mingw/build.sh - MinGW交叉编译主脚本

**用途**: 这是QGIS Windows版本的主要构建脚本，使用MinGW工具链进行交叉编译。

**主要功能**:
- 支持x86_64和i686两种架构
- 支持Debug和Release两种构建模式
- 自动处理依赖项链接
- 生成可移植的Windows安装包

**使用方法**:
```bash
# 在Docker容器中运行（推荐）
docker run --rm -w /QGIS -v $(pwd):/QGIS elpaso/qgis-deps-mingw:latest /QGIS/ms-windows/mingw/build.sh

# 直接运行
./ms-windows/mingw/build.sh [架构] [模式] [并行任务数]
# 架构: x86_64 (默认) 或 i686
# 模式: release (默认) 或 debug
# 并行任务数: 默认为 CPU核心数 * 1.5
```

**关键特性**:
- **架构支持**: 支持32位(i686)和64位(x86_64)构建
- **优化级别**: Release模式使用-O2优化，Debug模式使用-O0
- **依赖管理**: 自动检测和链接所需的DLL依赖
- **调试信息**: 自动提取调试符号到.debug文件
- **插件支持**: 包含Qt插件、GDAL插件等
- **Python集成**: 完整的Python环境和库支持

**构建配置**:
- 禁用3D支持 (`-DWITH_3D=OFF`)
- 启用服务器模式 (`-DWITH_SERVER=ON`)
- 启用服务器登录页面 (`-DWITH_SERVER_LANDINGPAGE_WEBAPP=ON`)
- 启用Quick支持 (`-DWITH_QUICK=ON`)
- 全局Python绑定安装 (`-DBINDINGS_GLOBAL_INSTALL=ON`)

**输出文件**:
- `qgis-portable-win64.zip`: 可移植的Windows 64位版本
- `qgis-portable-win64-debugsym.zip`: 调试符号文件

### 2. mingw/mingwdeps.sh - MinGW依赖安装脚本

**用途**: 在Fedora系统上安装MinGW交叉编译所需的所有依赖包。

**主要功能**:
- 配置DNF包管理器
- 安装MinGW64工具链
- 安装QGIS所需的所有库依赖
- 安装Python包和Qt组件

**依赖分类**:

**核心工具**:
- `mingw64-gcc-c++`: C++编译器
- `mingw64-gdb`: 调试器
- `ccache`: 编译缓存工具

**地理空间库**:
- `mingw64-gdal`: 地理空间数据抽象库
- `mingw64-geos`: 几何计算库
- `mingw64-proj`: 坐标系统转换库
- `mingw64-spatialindex`: 空间索引库
- `mingw64-GeographicLib`: 地理计算库

**Qt框架**:
- `mingw64-qt5-qtbase`: Qt基础模块
- `mingw64-qt5-qtwebkit`: Web浏览器组件
- `mingw64-qt5-qtlocation`: 位置服务
- `mingw64-qt5-qtmultimedia`: 多媒体支持
- `mingw64-qt5-qtsvg`: SVG支持

**Python环境**:
- `mingw64-python3`: Python解释器
- `mingw64-python3-qt5`: PyQt5绑定
- `mingw64-python3-gdal`: GDAL Python绑定
- `mingw64-python3-numpy`: 数值计算库
- `mingw64-python3-psycopg2`: PostgreSQL连接器

**数据库支持**:
- `mingw64-postgresql`: PostgreSQL客户端
- `mingw64-sqlite`: SQLite数据库

### 3. cygwin/package.sh - Cygwin构建脚本

**用途**: 在Cygwin环境下原生编译QGIS的脚本。

**主要功能**:
- 创建构建目录
- 配置CMake参数
- 执行编译、测试和安装

**配置特点**:
- **构建名称**: "cygwin"
- **站点标识**: "qgis.org"
- **严格模式**: 启用(`PEDANTIC=TRUE`)
- **服务器支持**: 启用(`WITH_SERVER=TRUE`)
- **SpatiaLite支持**: 启用(`WITH_SPATIALITE=TRUE`)
- **测试**: 启用(`ENABLE_TESTS=YES`)
- **安装路径**: `/usr`

**禁用功能**:
- GRASS插件 (`WITH_GRASS=FALSE`)
- Oracle支持 (`WITH_ORACLE=FALSE`)

**特殊配置**:
- 设置PyQt4工具路径
- 启用自定义小部件支持
- 配置Cygwin特定的Win32兼容性

### 4. dev/qgis.env.in - 开发环境模板

**用途**: 为基于vcpkg的Windows开发环境提供环境变量配置模板。

**配置变量**:

**PATH**: 设置可执行文件搜索路径
```
@VCPKG_TARGET_PATH@\bin;$ENV{WINDIR};$ENV{WINDIR}\system32\WBem
```

**PYTHONPATH**: 配置Python模块搜索路径
```
@QGIS_OUTPUT_DIRECTORY@\python;@VCPKG_TARGET_PATH@/tools/python3/Lib;@VCPKG_TARGET_PATH@/tools/python3/Lib/site-packages
```

**PROJ_DATA**: 设置PROJ坐标系统数据路径
```
@VCPKG_TARGET_PATH@/share/proj
```

**QT_PLUGIN_PATH**: 配置Qt插件路径
```
@VCPKG_TARGET_PATH@/Qt6/plugins;@VCPKG_TARGET_PATH@/bin/Qca
```

**PYTHONHOME**: 设置Python安装根目录
```
@VCPKG_TARGET_PATH@\tools\python3
```

**使用说明**:
- 模板中的`@VCPKG_TARGET_PATH@`和`@QGIS_OUTPUT_DIRECTORY@`会在构建时被实际路径替换
- 此配置确保QGIS能够正确找到所有依赖库和插件
- 支持Qt6环境下的开发和调试

## 构建流程对比

### MinGW交叉编译 (推荐)
- **优点**: 
  - 在Linux上构建Windows版本
  - 构建环境稳定可重现
  - 支持Docker容器化
  - 自动化程度高
- **缺点**: 
  - 需要Linux环境
  - 交叉编译复杂性

### Cygwin原生编译
- **优点**: 
  - 在Windows上直接构建
  - 原生Windows环境
- **缺点**: 
  - 环境配置复杂
  - 依赖管理困难
  - 构建时间较长

### vcpkg开发环境
- **优点**: 
  - 现代化依赖管理
  - 与Visual Studio集成好
  - 支持Qt6
- **缺点**: 
  - 仅适合开发环境
  - 不产生发布包

## 持续集成使用

这些脚本被广泛用于QGIS的持续集成系统：

1. **GitHub Actions**: 每次Pull Request都会触发MinGW构建
2. **夜间构建**: 定期生成Windows开发版本
3. **发布构建**: 用于生成正式发布的Windows安装包

## 维护和更新

- **依赖更新**: 定期更新`mingwdeps.sh`中的包列表
- **构建优化**: 根据性能需求调整编译参数
- **新功能支持**: 随QGIS功能更新修改CMake配置
- **环境兼容性**: 确保与最新的MinGW和vcpkg版本兼容

这些工具为QGIS提供了完整的Windows平台构建解决方案，支持从开发调试到最终发布的全流程。