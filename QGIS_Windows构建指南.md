# QGIS Windows平台编译打包详细指南

本文档基于QGIS项目官方构建系统分析，提供Windows平台下编译QGIS安装版和直接运行版的完整指南。

## 目录

- [构建方法概述](#构建方法概述)
- [方法一：MinGW交叉编译（推荐）](#方法一mingw交叉编译推荐)
- [方法二：Windows原生vcpkg构建](#方法二windows原生vcpkg构建)
- [方法三：Visual Studio + OSGeo4W](#方法三visual-studio--osgeo4w)
- [方法四：Cygwin原生编译](#方法四cygwin原生编译)
- [输出产物说明](#输出产物说明)
- [故障排除](#故障排除)

---

## 构建方法概述

QGIS支持以下四种Windows构建方法，每种方法适用于不同的使用场景：

| 构建方法 | 适用场景 | 优点 | 缺点 | 输出类型 |
|---------|---------|------|------|---------|
| MinGW交叉编译 | CI/CD、批量构建 | 环境一致、自动化程度高 | 需要Linux环境 | 便携版ZIP |
| vcpkg构建 | 开发调试 | 现代化、与VS集成好 | 仅适合开发 | 开发版本 |
| OSGeo4W构建 | 官方发布 | 官方支持、功能完整 | 配置复杂 | MSI安装包 |
| Cygwin编译 | 特殊需求 | Windows原生 | 维护困难 | 原生版本 |

---

## 方法一：MinGW交叉编译（推荐）

### 概述
这是QGIS官方CI/CD系统使用的主要方法，可以在Linux环境下交叉编译生成Windows可执行文件。

### 环境要求
- **操作系统**: Linux (推荐Fedora)
- **Docker**: 支持Docker容器化构建
- **CPU**: 多核心处理器（并行编译）
- **内存**: 建议8GB以上
- **存储**: 至少10GB可用空间

### 快速开始（Docker方式）

#### 1. 克隆源代码
```bash
# 克隆QGIS源代码
git clone https://github.com/qgis/QGIS.git
cd QGIS
```

#### 2. 使用Docker构建
```bash
# 64位Release版本（推荐）
docker run --rm -w /QGIS -v $(pwd):/QGIS elpaso/qgis-deps-mingw:latest /QGIS/ms-windows/mingw/build.sh

# 64位Debug版本
docker run --rm -w /QGIS -v $(pwd):/QGIS elpaso/qgis-deps-mingw:latest /QGIS/ms-windows/mingw/build.sh x86_64 debug

# 32位版本
docker run --rm -w /QGIS -v $(pwd):/QGIS elpaso/qgis-deps-mingw:latest /QGIS/ms-windows/mingw/build.sh i686
```

#### 3. 构建输出
构建完成后会在项目根目录生成：
- `qgis-portable-win64.zip`: 64位便携版
- `qgis-portable-win64-debugsym.zip`: 调试符号文件

### 本地构建（无Docker）

#### 1. 安装依赖（Fedora系统）
```bash
# 执行依赖安装脚本
sudo ./ms-windows/mingw/mingwdeps.sh
```

该脚本将安装以下关键组件：
- **编译工具**: mingw64-gcc-c++, mingw64-gdb, ccache
- **地理空间库**: mingw64-gdal, mingw64-geos, mingw64-proj, mingw64-spatialindex
- **Qt框架**: mingw64-qt5-qtbase, mingw64-qt5-qtwebkit, mingw64-qt5-qtsvg等
- **Python环境**: mingw64-python3, mingw64-python3-qt5, mingw64-python3-gdal等
- **数据库支持**: mingw64-postgresql, mingw64-sqlite

#### 2. 执行构建
```bash
# 基础构建命令
./ms-windows/mingw/build.sh [架构] [模式] [并行任务数]

# 参数说明：
# 架构: x86_64 (默认) 或 i686
# 模式: release (默认) 或 debug  
# 并行任务数: 默认为 CPU核心数 * 1.5

# 示例
./ms-windows/mingw/build.sh x86_64 release 8
```

### 构建配置详解

构建脚本使用以下CMake配置：

```cmake
# 主要配置选项
-DCMAKE_CROSS_COMPILING=1           # 启用交叉编译
-DUSE_CCACHE=ON                     # 启用编译缓存
-DCMAKE_BUILD_TYPE=$buildtype       # 构建类型 (Release/Debug)
-DBUILD_TESTING=OFF                 # 禁用测试构建
-DWITH_3D=OFF                       # 禁用3D功能
-DWITH_SERVER=ON                    # 启用服务器功能
-DWITH_SERVER_LANDINGPAGE_WEBAPP=ON # 启用服务器Web界面
-DWITH_QUICK=ON                     # 启用Quick支持
-DBINDINGS_GLOBAL_INSTALL=ON        # 全局Python绑定安装
```

### 自定义构建

#### 1. 修改构建选项
编辑 `ms-windows/mingw/build.sh`，调整CMake配置：

```bash
# 启用3D功能
-DWITH_3D=ON \
-DWITH_DRACO=ON \

# 启用PDAL点云支持
-DWITH_PDAL=ON \

# 启用Oracle支持
-DWITH_ORACLE=ON \
```

#### 2. 添加自定义依赖
如需添加额外依赖，修改 `ms-windows/mingw/mingwdeps.sh`：

```bash
# 添加新的mingw64包
mingw64-your-library \
mingw64-your-library-devel \
```

---

## 方法二：Windows原生vcpkg构建

### 概述
使用Microsoft的vcpkg包管理器进行现代化的Windows原生构建，特别适合开发环境。

### 环境要求
- **操作系统**: Windows 10/11
- **Visual Studio**: 2022 Community Edition或更高版本
- **vcpkg**: 最新版本
- **Git**: 用于获取源代码
- **PowerShell**: 用于运行构建脚本

### 构建步骤

#### 1. 安装工具

**安装Visual Studio 2022**
- 下载[Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/)
- 安装时选择"使用C++的桌面开发"工作负载

**安装winflexbison**
- 从[GitHub releases](https://github.com/lexxmark/winflexbison/releases)下载
- 解压到系统PATH中

#### 2. 方式A：使用预构建SDK（推荐）

```powershell
# 下载QGIS源代码
git clone https://github.com/qgis/QGIS.git
cd QGIS

# 下载最新的QGIS master SDK
# 从 https://nightly.link/qgis/QGIS/workflows/windows-qt6/master/qgis-sdk-x64-windows.zip 下载并解压

# 配置项目
cmake -S . `
      -B build `
      -DSDK_PATH="path/to/vcpkg-export-[date]" `
      -DBUILD_WITH_QT6=ON `
      -DWITH_QTWEBKIT=OFF `
      -DVCPKG_TARGET_TRIPLET=x64-windows-release `
      -DFLEX_EXECUTABLE="path/to/flex.exe" `
      -DBISON_EXECUTABLE="path/to/bison.exe"

# 构建项目
cmake --build build --config Release
```

#### 3. 方式B：本地构建所有依赖

```powershell
# 安装和初始化vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 返回QGIS目录
cd ..\QGIS

# 配置项目（会自动构建所有依赖）
cmake -S . `
      -B build `
      -D WITH_VCPKG=ON `
      -D BUILD_WITH_QT6=ON `
      -D WITH_QTWEBKIT=OFF `
      -D VCPKG_TARGET_TRIPLET=x64-windows-release `
      -D VCPKG_HOST_TRIPLET=x64-windows-release

# 构建项目
cmake --build build --config Release
```

### vcpkg依赖配置

QGIS的vcpkg依赖定义在 `vcpkg/vcpkg.json` 中：

```json
{
  "dependencies": [
    "duckdb",
    {"name": "exiv2", "features": ["xmp"]},
    "expat",
    {"name": "gdal", "features": ["arrow-adbc", "freexl", "kea", "parquet", "poppler", "tools"]},
    "geos",
    "gsl",
    "libpq",
    "libspatialindex",
    "libspatialite",
    "libxml2",
    "libzip",
    "nlohmann-json",
    "pdal",
    "proj",
    "protobuf",
    {"name": "qca", "features": ["ossl"]},
    {"name": "qtbase", "features": ["sql-odbc"]},
    "qtdeclarative",
    "qtimageformats",
    "qtkeychain-qt6",
    "qtlocation",
    "qtsvg",
    "qttools",
    "zlib"
  ]
}
```

### 开发环境配置

构建完成后，系统会自动生成 `build/output/bin/qgis.env` 环境配置文件：

```batch
PATH=C:\vcpkg\installed\x64-windows-release\bin;%WINDIR%;%WINDIR%\system32\WBem
PYTHONPATH=C:\QGIS\build\output\python;C:\vcpkg\installed\x64-windows-release\tools\python3\Lib
PROJ_DATA=C:\vcpkg\installed\x64-windows-release\share\proj
QT_PLUGIN_PATH=C:\vcpkg\installed\x64-windows-release\Qt6\plugins
PYTHONHOME=C:\vcpkg\installed\x64-windows-release\tools\python3
```

#### 使用PowerShell加载环境：
```powershell
# 加载环境变量
Get-Content build\output\bin\qgis.env | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}

# 启动QGIS
.\build\output\bin\qgis.exe
```

---

## 方法三：Visual Studio + OSGeo4W

### 概述
这是QGIS官方Windows发布版本使用的构建方法，可以生成MSI安装包。

### 环境要求
- **Windows**: 10/11 64位
- **Visual Studio**: 2022 Community Edition
- **OSGeo4W**: 最新版本
- **Cygwin**: 用于提供Unix工具

### 详细步骤

#### 1. 安装基础工具

**Visual Studio 2022 Community Edition**
- 下载并安装，选择"使用C++的桌面开发"

**CMake**
- 下载[CMake](https://github.com/Kitware/CMake/releases/latest) Windows安装程序

**Cygwin 64位**
- 下载[cygwin setup](https://cygwin.com/setup-x86_64.exe)
- 安装以下包：`bison`, `flex`, `git`

**ninja构建工具**
- 下载[ninja](https://github.com/ninja-build/ninja/releases/latest)
- 将 `ninja.exe` 复制到 `C:\OSGeo4W\bin\`

#### 2. 安装OSGeo4W

**下载OSGeo4W**
- 从[OSGeo4W官网](https://download.osgeo.org/osgeo4w/v2/osgeo4w-setup.exe)下载安装程序

**安装QGIS开发依赖**
- 运行OSGeo4W安装程序，选择"高级安装"
- 选择包：`qgis-dev-deps`
- **注意**: 不要安装 `msinttypes` 包，会导致编译冲突

#### 3. 获取源代码

```cmd
cd C:\OSGeo4W
git clone https://github.com/qgis/QGIS.git
cd QGIS
git config core.filemode false
```

#### 4. 官方构建方式（使用OSGeo4W构建配方）

```cmd
# 创建构建目录
mkdir osgeo4w-build
cd osgeo4w-build

# 下载官方构建脚本
curl -JLO https://raw.githubusercontent.com/jef-n/OSGeo4W/refs/heads/master/bootstrap.cmd
curl -JLO https://raw.githubusercontent.com/jef-n/OSGeo4W/refs/heads/master/bootstrap.sh

# 构建开发版
bootstrap.cmd qgis-dev

# 或构建其他版本
bootstrap.cmd qgis-ltr     # 长期支持版
bootstrap.cmd qgis         # 最新稳定版
```

构建成功后，OSGeo4W包将在 `x86_64/` 目录中。

#### 5. 手动配置构建（可选）

如果需要自定义配置：

```cmd
# 打开Visual Studio开发者命令提示符
# 配置构建
mkdir build
cd build

cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
      -DWITH_3D=ON ^
      -DWITH_SERVER=ON ^
      -DENABLE_TESTS=OFF ^
      -DCMAKE_PREFIX_PATH=C:\OSGeo4W ^
      ..

# 构建
cmake --build . --config RelWithDebInfo --parallel %NUMBER_OF_PROCESSORS%
```

#### 6. 创建MSI安装包

OSGeo4W构建系统会自动处理MSI创建，相关脚本：
- `scripts\msis.sh`: MSI创建脚本
- `scripts/creatmsi.pl`: MSI创建核心逻辑

---

## 方法四：Cygwin原生编译

### 概述
在Cygwin环境下直接编译QGIS，适用于需要完全Windows原生环境的特殊场景。

### 环境准备

#### 1. 安装Cygwin
- 下载[Cygwin 64位安装程序](https://cygwin.com/setup-x86_64.exe)
- 安装必要的开发包：
  - `gcc-g++`
  - `cmake`
  - `make`
  - `git`
  - `python3`
  - `python3-devel`

#### 2. 安装Qt和其他依赖
```bash
# 在Cygwin终端中安装包
apt-cyg install qt5-devel
apt-cyg install gdal-devel
apt-cyg install geos-devel
apt-cyg install proj-devel
apt-cyg install sqlite3-devel
```

### 构建步骤

#### 1. 获取源代码
```bash
git clone https://github.com/qgis/QGIS.git
cd QGIS
```

#### 2. 使用官方构建脚本
```bash
# 执行Cygwin构建脚本
./ms-windows/cygwin/package.sh
```

#### 3. 手动构建（可选）
```bash
mkdir build
cd build

# 配置构建
cmake -D CMAKE_BUILD_TYPE=Release \
      -D WITH_SERVER=TRUE \
      -D WITH_SPATIALITE=TRUE \
      -D WITH_QSPATIALITE=TRUE \
      -D ENABLE_TESTS=YES \
      -D CMAKE_INSTALL_PREFIX=/usr \
      ..

# 编译
make -j$(nproc)

# 运行测试
make test

# 安装
make install
```

---

## 输出产物说明

### MinGW交叉编译输出

#### qgis-portable-win64.zip 内容结构：
```
qgis-portable-win64/
├── bin/                    # 可执行文件和DLL
│   ├── qgis.exe           # QGIS主程序
│   ├── qgis-server.exe    # QGIS服务器
│   ├── *.dll              # 依赖库
│   └── plugins/           # Qt插件
├── share/                  # 数据文件
│   ├── qgis/              # QGIS资源
│   ├── gdal/              # GDAL数据
│   └── proj/              # 坐标系统数据
├── lib/                   # 库文件
│   ├── python3.x/         # Python环境
│   └── qgis/              # QGIS模块
└── origins.txt            # 依赖来源信息
```

#### qgis-portable-win64-debugsym.zip：
- 包含所有可执行文件的调试符号（.debug文件）
- 用于程序调试和崩溃分析

### vcpkg构建输出

#### 开发版本结构：
```
build/output/
├── bin/
│   ├── qgis.exe           # QGIS可执行文件
│   ├── qgis.env           # 环境配置文件
│   └── *.dll              # 核心库
├── lib/                   # 静态库和模块
├── share/                 # 资源文件
└── python/                # Python绑定
```

### OSGeo4W构建输出

#### OSGeo4W包结构：
```
x86_64/
├── qgis-dev-*.tar.xz      # QGIS开发版包
├── setup.ini              # 包管理信息
└── *.msi                  # MSI安装程序（如果生成）
```

---

## 故障排除

### 常见问题及解决方案

#### 1. MinGW编译问题

**问题：Docker镜像拉取失败**
```bash
# 解决方案：使用镜像加速器
docker pull registry.cn-hangzhou.aliyuncs.com/elpaso/qgis-deps-mingw:latest
```

**问题：依赖库缺失**
```bash
# 检查mingw依赖安装
sudo dnf list installed | grep mingw64

# 重新安装依赖
sudo ./ms-windows/mingw/mingwdeps.sh
```

**问题：编译内存不足**
```bash
# 减少并行编译任务数
./ms-windows/mingw/build.sh x86_64 release 2
```

#### 2. vcpkg构建问题

**问题：vcpkg依赖构建失败**
```powershell
# 清除vcpkg缓存
.\vcpkg\vcpkg remove --outdated

# 更新vcpkg到最新版本
cd vcpkg
git pull
.\bootstrap-vcpkg.bat
```

**问题：环境变量未生效**
```powershell
# 手动检查qgis.env文件
Get-Content build\output\bin\qgis.env

# 验证路径是否存在
Test-Path "C:\vcpkg\installed\x64-windows-release\bin"
```

#### 3. OSGeo4W构建问题

**问题：Visual Studio找不到**
```cmd
# 使用VS开发者命令提示符
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

**问题：OSGeo4W依赖冲突**
```cmd
# 清理OSGeo4W安装
# 重新运行OSGeo4W安装程序，选择清理安装
```

#### 4. 运行时问题

**问题：DLL缺失**
- **MinGW版本**: 检查依赖是否正确打包
- **vcpkg版本**: 确认环境变量配置正确
- **OSGeo4W版本**: 检查OSGeo4W环境变量

**问题：Python插件无法加载**
```bash
# 检查Python路径
echo $PYTHONPATH

# 检查Python版本兼容性
python --version
```

**问题：坐标系统错误**
```bash
# 检查PROJ数据
echo $PROJ_DATA
ls -la "$PROJ_DATA"
```

### 性能优化建议

#### 1. 编译优化
- **并行编译**: 设置适当的并行任务数 (`-j` 参数)
- **编译缓存**: 启用ccache加速重复编译
- **内存管理**: 确保有足够的RAM避免swap

#### 2. 存储优化
- **SSD存储**: 使用SSD存储源代码和构建目录
- **空间管理**: 定期清理中间文件
- **分离构建**: 将构建目录与源代码分离

#### 3. 依赖管理
- **镜像使用**: 使用本地或加速镜像
- **版本固定**: 固定依赖版本避免兼容性问题
- **增量构建**: 利用构建系统的增量特性

---

## 总结

本指南提供了四种不同的QGIS Windows构建方法：

1. **MinGW交叉编译**: 适合CI/CD和批量构建，输出便携版
2. **vcpkg构建**: 适合现代Windows开发环境
3. **OSGeo4W构建**: 官方发布方式，可生成MSI安装包
4. **Cygwin编译**: 特殊需求的原生Windows构建

选择适合你需求的方法，按照步骤执行即可获得Windows版本的QGIS安装包或运行版本。

对于一般用户，推荐使用MinGW交叉编译方法，因为它配置简单、环境一致且自动化程度高。对于开发者，推荐使用vcpkg方法，因为它与现代开发工具集成更好。