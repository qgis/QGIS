# QGIS Docker 目录分析

## 目录概述

QGIS项目的`.docker`目录包含了完整的Docker化构建、测试和CI/CD基础设施。这个目录是QGIS项目实现自动化构建、持续集成和跨平台测试的核心组件。

## 目录结构分析

### 核心文件

```
.docker/
├── README.md                              # Docker使用说明文档
├── docker-variables.env                   # 环境变量配置文件
├── qgis.dockerfile                        # 主QGIS镜像Dockerfile
├── qgis3-qt5-build-deps.dockerfile        # Qt5依赖镜像Dockerfile  
├── qgis3-qt6-build-deps.dockerfile        # Qt6依赖镜像Dockerfile
├── docker-compose-testing.yml             # 测试环境编排文件
├── docker-compose-testing-postgres.yml    # PostgreSQL测试环境
├── docker-compose-testing-oracle.yml      # Oracle测试环境
├── docker-compose-testing-mssql.yml       # MSSQL测试环境
├── docker-qgis-build.sh                   # QGIS构建脚本
├── docker-qgis-test.sh                    # QGIS测试脚本
├── docker-qgis-clangtidy.sh              # 静态代码分析脚本
├── qgis_resources/                        # QGIS资源文件目录
│   ├── requirements.txt                   # Python依赖列表
│   ├── test_runner/                       # 测试运行器
│   │   ├── qgis_testrunner.sh            # 测试运行脚本
│   │   ├── qgis_testrunner.py            # 测试运行器Python实现
│   │   ├── qgis_setup.sh                 # QGIS环境设置脚本
│   │   └── qgis_startup.py               # QGIS启动脚本
│   └── supervisor/                        # 进程管理配置
│       ├── supervisord.conf              # Supervisor主配置
│       └── supervisor.d/                 # Supervisor服务配置
│           └── supervisor.xvfb.conf      # Xvfb虚拟显示配置
└── webdav/                               # WebDAV测试服务配置
    ├── nginx.conf                        # Nginx配置
    └── passwords.list                    # 认证密码列表
```

## 主要组件功能分析

### 1. Docker镜像体系

#### 依赖镜像（qgis3-build-deps）
- **作用**: 包含编译QGIS所需的所有依赖项
- **版本**: 支持Qt5和Qt6两个版本
- **基础**: 基于Ubuntu LTS版本
- **内容**: 编译工具链、地理空间库、Qt框架等

#### 主镜像（qgis/qgis）
- **作用**: 包含完整编译后的QGIS应用程序
- **功能**: 
  - 运行QGIS桌面应用
  - 执行单元测试
  - 提供无头测试环境
- **特性**: 
  - 集成Xvfb虚拟显示服务器
  - 支持完全自动化的CI/CD流水线

### 2. 构建系统

#### docker-qgis-build.sh核心功能：

**编译配置**:
```bash
cmake \
 -GNinja \
 -DCMAKE_BUILD_TYPE=Release \
 -DUSE_CCACHE=ON \
 -DBUILD_WITH_QT6=${BUILD_WITH_QT6} \
 -DWITH_DESKTOP=ON \
 -DWITH_SERVER=ON \
 -DWITH_3D=ON \
 -DENABLE_TESTS=ON \
 -DWITH_BINDINGS=ON
```

**关键特性**:
- 使用ccache加速编译过程
- 支持Qt5/Qt6切换
- 启用完整功能集（桌面、服务器、3D）
- 集成静态代码分析（Clang-Tidy）
- 支持多种数据库后端测试

### 3. 测试框架

#### 测试环境编排
通过docker-compose管理复杂的测试环境：

```yaml
services:
  httpbin:      # HTTP测试服务
  webdav:       # WebDAV协议测试
  minio:        # S3兼容对象存储测试  
  qgis-deps:    # QGIS主测试容器
```

#### 测试执行流程
1. **环境准备**: `qgis_setup.sh` 配置QGIS测试环境
2. **插件安装**: 创建符号链接模拟插件安装
3. **测试执行**: `qgis_testrunner.sh` 运行具体测试
4. **结果收集**: 解析测试输出并返回适当的退出码

### 4. CI/CD集成

#### 支持的CI平台
- **GitHub Actions**: 通过环境变量和工作流集成
- **Travis CI**: 提供完整的配置示例
- **Circle-CI**: 支持原生Docker工作流

#### 测试类型
- **单元测试**: Python和C++单元测试
- **集成测试**: 数据库连接、网络服务测试
- **插件测试**: 第三方QGIS插件测试支持
- **静态分析**: Clang-Tidy代码质量检查
