# CLAUDE.md

此文件为Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

- 所有对话和文档都使用中文
- 文档使用 markdown 格式

## QGIS项目概述

QGIS是一个功能齐全的开源地理信息系统(GIS)，使用C++和Qt框架构建，支持Python插件和脚本。项目结构复杂，包含多个模块和层次。

## 核心架构

### 源码结构
- **src/core/** - QGIS核心库，包含基础地理空间功能、数据模型和渲染引擎
- **src/gui/** - GUI组件和用户界面小部件
- **src/app/** - 主应用程序和桌面界面
- **src/3d/** - 3D可视化和渲染功能
- **src/analysis/** - 分析算法和地理处理工具
- **src/auth/** - 认证框架
- **python/** - Python绑定和脚本接口

### 关键组件
- **地图画布（Map Canvas）**: 主要的地图显示组件 (src/gui/qgsmapcanvas.*)
- **数据提供者**: 处理不同数据源格式的抽象接口
- **图层系统**: 栅格、矢量、网格和点云图层的统一接口
- **符号系统**: 复杂的地图符号化和渲染引擎
- **处理框架**: 地理处理算法的插件架构

## 构建命令

### 基础构建
```bash
mkdir build && cd build
cmake -D ENABLE_TESTS=ON ..
make -j$(nproc)
```

### 构建选项
- **启用测试**: `cmake -D ENABLE_TESTS=ON ..`
- **启用PostgreSQL测试**: `cmake -D ENABLE_TESTS=ON -D ENABLE_PGTEST=ON ..`
- **3D支持**: 默认启用 (`WITH_3D=TRUE`)
- **Python支持**: 默认启用 (`WITH_PYTHON=ON`)

### 构建依赖管理
- **系统库**: 默认使用系统库 (`PREFER_INTERNAL_LIBS=TRUE`)
- **VCPKG**: 可选择使用vcpkg进行依赖管理 (`WITH_VCPKG=ON`)

## 测试

### 运行所有测试
```bash
make check  # 需要xvfb-run
```

### 运行单个测试
```bash
ctest -V -R PyQgsFeature
```

### 无显示器环境运行测试
```bash
xvfb-run --server-args=-screen\ 0\ 1024x768x24 ctest -V -R TestName
```

### Python测试环境设置
```bash
source build/tests/env.sh
python tests/src/python/test_provider_postgres.py TestPyQgsPostgresProvider.testExtent
```

### 测试数据库设置（PostgreSQL）
```bash
# 使用Docker设置测试数据库
QGIS_WORKSPACE=${srcdir} docker compose -f .docker/docker-compose-testing-postgres.yml up -d postgres
tests/testdata/provider/testdata_pg.sh
```

## 开发工作流

### 代码规范
- **预提交钩子**: 安装 `pre-commit install --install-hooks` (需要版本4.1+)
- **代码风格**: 使用项目配置的astyle进行格式化
- **提交信息**: 新功能使用`[FEATURE]`标签

### 通用开发任务

#### C++开发
- 头文件位于 `src/*/` 目录
- 遵循Qt编码约定和QGIS特定模式
- 使用SIP注释为Python生成绑定

#### Python开发
- Python绑定位于 `python/` 目录
- 使用PyQGIS API进行脚本开发
- 处理算法在 `python/plugins/processing/`

#### 3D功能开发
- 3D相关代码在 `src/3d/`
- 使用Qt3D框架
- 支持多种3D渲染器和材质

#### GUI组件开发
- 自定义组件在 `src/customwidgets/`
- UI文件和对话框在相应模块目录
- 遵循Qt Designer约定

### 插件和扩展
- 插件系统支持C++和Python
- 数据提供者插件需要实现特定接口
- 处理算法通过处理框架集成

## 重要配置文件
- **CMakeLists.txt**: 主构建配置
- **vcpkg/vcpkg.json**: 依赖管理配置
- **python/requirements.txt**: Python依赖
- **.pre-commit-config.yaml**: 代码质量检查配置

## 依赖关系
- **Qt**: 5.x或6.x (GUI框架)
- **GDAL/OGR**: 地理空间数据访问
- **GEOS**: 几何运算
- **PROJ**: 坐标系统转换
- **SpatiaLite/PostgreSQL**: 空间数据库支持
- **Python**: 脚本和插件支持

## 调试和分析
- **GDB调试**: 支持Python和C++混合调试
- **渲染检查器**: 用于地图渲染测试的工具
- **性能分析**: 支持运行时性能分析

## 发布和打包
- **Debian包**: `scripts/build_debian_package.sh`
- **Windows**: MinGW64交叉编译支持
- **macOS**: Mac打包器和依赖管理
- **Docker**: 容器化构建和测试环境