# QGIS 初级C++开发指南

本文档专为刚接触QGIS项目的初级C++工程师准备，帮助你快速上手QGIS开发。

## 1. 项目概览

QGIS是一个大型的地理信息系统(GIS)软件，使用C++和Qt框架开发。项目有以下特点：

- **代码规模**: 超过100万行C++代码
- **架构**: 模块化设计，分为核心库、GUI、应用程序等多个层次
- **技术栈**: C++17、Qt5/Qt6、CMake、Python
- **开发模式**: 开源协作，严格的代码规范和测试要求

## 2. 开发环境搭建

### 2.1 系统要求

#### Linux (Ubuntu/Debian推荐)
```bash
# 基础编译工具
sudo apt-get install build-essential

# CMake (最低版本3.22.0)
sudo apt-get install cmake

# Qt开发库 (最低Qt 5.15.2)
sudo apt-get install qtbase5-dev qttools5-dev-tools

# Python支持 (最低Python 3.11)
sudo apt-get install python3-dev python3-pip
```

#### 核心依赖库
```bash
# 地理空间处理库
sudo apt-get install libgdal-dev libproj-dev libgeos-dev libspatialite-dev

# Qt相关库
sudo apt-get install libqt5svg5-dev libqwt-qt5-dev libqca-qt5-2-dev

# 其他依赖
sudo apt-get install libzip-dev libexpat1-dev libsqlite3-dev
```

### 2.2 IDE配置建议

#### Qt Creator (推荐)
- 完美支持Qt项目
- 内置CMake支持
- 优秀的C++智能提示
- 集成调试器

#### VSCode
- 安装C/C++扩展
- 安装CMake扩展
- 配置clangd语言服务器

#### CLion
- 商业IDE，功能强大
- 原生CMake支持
- 优秀的重构工具

## 3. 构建项目

### 3.1 获取源码
```bash
git clone https://github.com/qgis/QGIS.git
cd QGIS
```

### 3.2 创建构建目录
```bash
mkdir build
cd build
```

### 3.3 配置构建
```bash
# 基础配置
cmake -D ENABLE_TESTS=ON ..

# 开发配置（推荐）
cmake \
  -D CMAKE_BUILD_TYPE=Debug \
  -D ENABLE_TESTS=ON \
  -D WITH_3D=ON \
  -D WITH_PYTHON=ON \
  -D CMAKE_INSTALL_PREFIX=../install \
  ..
```

### 3.4 编译
```bash
# 使用所有CPU核心编译
make -j$(nproc)

# 或只编译核心库（更快）
make qgis_core -j$(nproc)
```

### 3.5 安装（可选）
```bash
make install
```

## 4. 项目架构深入理解

### 4.1 源码目录结构
```
src/
├── core/           # 核心库 - 不依赖GUI的基础功能
├── gui/            # GUI组件库 - 可重用的界面组件
├── app/            # 桌面应用程序 - QGIS主程序
├── 3d/             # 3D可视化库
├── analysis/       # 分析算法库
├── auth/           # 认证框架
└── providers/      # 数据提供者插件
```

### 4.2 核心类层次结构

#### QgsMapLayer家族
```cpp
QgsMapLayer (基类)
├── QgsVectorLayer (矢量图层)
├── QgsRasterLayer (栅格图层)
├── QgsMeshLayer (网格图层)
└── QgsPointCloudLayer (点云图层)
```

#### QgsFeature数据模型
```cpp
QgsFeature      // 地理要素
├── geometry()  // 几何信息
├── attributes() // 属性数据
└── id()        // 唯一标识
```

### 4.3 重要的设计模式

#### 1. 工厂模式
```cpp
// 数据提供者工厂
QgsProviderRegistry::instance()->createProvider(...)
```

#### 2. 观察者模式
```cpp
// 图层状态变化通知
connect(layer, &QgsMapLayer::dataChanged, this, &MyClass::onDataChanged);
```

#### 3. 单例模式
```cpp
// 应用程序单例
QgsApplication::instance()
```

## 5. 第一个简单开发任务

### 5.1 创建新的分析工具

假设你要创建一个计算图层要素数量的工具：

```cpp
// qgsfeaturecounter.h
#ifndef QGSFEATURECOUNTER_H
#define QGSFEATURECOUNTER_H

#include "qgis_analysis.h"
#include <QObject>

class QgsVectorLayer;

class ANALYSIS_EXPORT QgsFeatureCounter : public QObject
{
    Q_OBJECT

  public:
    explicit QgsFeatureCounter(QObject *parent = nullptr);
    
    /**
     * 计算图层中的要素数量
     */
    long long countFeatures(QgsVectorLayer *layer);

  signals:
    void countCompleted(long long count);

  private:
    // 私有成员变量
};

#endif // QGSFEATURECOUNTER_H
```

```cpp
// qgsfeaturecounter.cpp
#include "qgsfeaturecounter.h"
#include "qgsvectorlayer.h"
#include "qgsfeatureiterator.h"

QgsFeatureCounter::QgsFeatureCounter(QObject *parent)
  : QObject(parent)
{
}

long long QgsFeatureCounter::countFeatures(QgsVectorLayer *layer)
{
  if (!layer || !layer->isValid())
    return -1;

  // 使用QGIS的特征迭代器
  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature feature;
  long long count = 0;

  while (it.nextFeature(feature))
  {
    ++count;
  }

  emit countCompleted(count);
  return count;
}
```

### 5.2 添加到构建系统

在相应的CMakeLists.txt中添加：
```cmake
set(ANALYSIS_SRCS
  # ... 现有文件 ...
  qgsfeaturecounter.cpp
)

set(ANALYSIS_HDRS
  # ... 现有文件 ...
  qgsfeaturecounter.h
)
```

## 6. 常用开发模式

### 6.1 错误处理
```cpp
// QGIS风格的错误处理
if (!layer->isValid())
{
  QgsDebugMsg("图层无效");
  return false;
}
```

### 6.2 内存管理
```cpp
// 使用智能指针
std::unique_ptr<QgsGeometry> geom = std::make_unique<QgsGeometry>();

// 或QGIS的对象生存期管理
QgsVectorLayer *layer = new QgsVectorLayer();
layer->setParent(this); // Qt对象树管理
```

### 6.3 信号槽机制
```cpp
// 连接信号槽
connect(layer, &QgsMapLayer::nameChanged, 
        this, &MyClass::onLayerNameChanged);

// 自定义信号槽
private slots:
  void onLayerNameChanged(const QString &name);
```

### 6.4 Python绑定
```cpp
// 在头文件中添加SIP注释
class ANALYSIS_EXPORT QgsFeatureCounter : public QObject
{
  // ... 类定义 ...
  
#ifdef SIP_RUN
  // Python特定的代码
#endif
};
```

## 7. 调试和测试

### 7.1 使用调试器
```bash
# 在GDB中运行QGIS
cd build
gdb ./output/bin/qgis

# 设置断点
(gdb) b QgsFeatureCounter::countFeatures
(gdb) run
```

### 7.2 日志调试
```cpp
// 使用QGIS日志系统
#include "qgslogger.h"

QgsDebugMsg("调试信息");
QgsDebugMsgLevel("详细调试信息", 2);
```

### 7.3 单元测试
```cpp
// testqgsfeaturecounter.cpp
#include <QTest>
#include "qgsfeaturecounter.h"
#include "qgsvectorlayer.h"

class TestQgsFeatureCounter : public QObject
{
    Q_OBJECT

  private slots:
    void testCountFeatures();
};

void TestQgsFeatureCounter::testCountFeatures()
{
  // 创建测试图层
  QgsVectorLayer layer("Point", "test", "memory");
  
  QgsFeatureCounter counter;
  long long count = counter.countFeatures(&layer);
  
  QCOMPARE(count, 0LL);
}

QTEST_MAIN(TestQgsFeatureCounter)
#include "testqgsfeaturecounter.moc"
```

### 7.4 运行测试
```bash
# 运行所有测试
make check

# 运行特定测试
ctest -V -R TestQgsFeatureCounter
```

## 8. 代码规范

### 8.1 命名约定
```cpp
// 类名：Pascal命名法，以Qgs开头
class QgsFeatureCounter

// 成员函数：camelCase
void countFeatures()

// 成员变量：m开头的camelCase
int mFeatureCount;

// 常量：全大写加下划线
static const int MAX_FEATURES = 1000;
```

### 8.2 代码格式化
```bash
# 安装pre-commit钩子
pip install pre-commit
pre-commit install --install-hooks

# 手动运行格式化
scripts/astyle.sh src/analysis/qgsfeaturecounter.cpp
```

### 8.3 文档注释
```cpp
/**
 * \ingroup analysis
 * \brief 计算矢量图层中的要素数量
 * 
 * 这个类提供了计算QgsVectorLayer中要素数量的功能。
 * 它使用QgsFeatureIterator来遍历所有要素。
 * 
 * \since QGIS 3.30
 */
class ANALYSIS_EXPORT QgsFeatureCounter
{
  public:
    /**
     * 计算指定图层中的要素数量
     * \param layer 要统计的矢量图层
     * \returns 要素数量，如果图层无效返回-1
     */
    long long countFeatures(QgsVectorLayer *layer);
};
```

## 9. 常见问题解决

### 9.1 编译错误
```bash
# 找不到头文件
# 解决：检查include路径，确保依赖库已安装

# 链接错误
# 解决：检查CMakeLists.txt中的库链接配置

# Qt MOC错误
# 解决：确保Q_OBJECT宏在类声明中，重新运行cmake
```

### 9.2 运行时问题
```bash
# 找不到动态库
export LD_LIBRARY_PATH=$PWD/output/lib:$LD_LIBRARY_PATH

# Python插件加载失败
export PYTHONPATH=$PWD/output/python:$PYTHONPATH
```

### 9.3 调试技巧
```cpp
// 使用Qt调试宏
Q_ASSERT(layer != nullptr);
qDebug() << "Layer name:" << layer->name();

// 条件编译调试代码
#ifdef QGIS_DEBUG
  qDebug() << "Debug mode enabled";
#endif
```

## 10. 进阶学习路径

### 10.1 深入了解核心概念
1. **坐标参考系统** (`src/core/qgscoordinatereferencesystem.h`)
2. **几何处理** (`src/core/geometry/`)
3. **渲染系统** (`src/core/symbology/`)
4. **数据提供者** (`src/providers/`)

### 10.2 学习GUI开发
1. **地图画布** (`src/gui/qgsmapcanvas.h`)
2. **自定义控件** (`src/customwidgets/`)
3. **对话框设计** (`src/ui/`)

### 10.3 插件开发
1. **C++插件** - 学习插件接口
2. **Python插件** - 使用PyQGIS API
3. **处理算法** - 编写地理处理工具

## 11. 有用的资源

### 11.1 官方文档
- [QGIS开发者指南](https://docs.qgis.org/latest/en/docs/developers_guide/)
- [PyQGIS API文档](https://qgis.org/pyqgis/)
- [C++ API文档](https://qgis.org/api/)

### 11.2 社区资源
- [QGIS GitHub仓库](https://github.com/qgis/QGIS)
- [开发者邮件列表](https://lists.osgeo.org/mailman/listinfo/qgis-developer)
- [QEP提案](https://github.com/qgis/QGIS-Enhancement-Proposals)

### 11.3 相关技术文档
- [Qt文档](https://doc.qt.io/)
- [CMake文档](https://cmake.org/documentation/)
- [GDAL/OGR文档](https://gdal.org/)

## 总结

QGIS是一个复杂但组织良好的项目。作为初级C++开发者，建议：

1. **从小做起** - 先熟悉构建系统和基本工具
2. **阅读现有代码** - 理解项目的编码风格和模式
3. **编写测试** - 确保代码质量
4. **积极参与社区** - 提问和贡献代码

记住，QGIS开发不仅仅是写C++代码，还涉及地理空间概念、Qt框架、Python集成等多个方面。保持耐心，持续学习！