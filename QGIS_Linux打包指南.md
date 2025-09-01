# QGIS Linux系统打包指南

## 概述

QGIS项目提供了完整的Linux系统打包解决方案，支持Debian/Ubuntu和Fedora/RHEL/CentOS系统。本文档详细介绍如何在不同Linux发行版上构建QGIS安装包。

## 系统要求

### 通用要求
- Git版本控制工具
- 完整的QGIS源代码（包含所有子模块）
- 足够的磁盘空间（建议10GB以上）

### Ubuntu/Debian系统要求
- Ubuntu 18.04+ 或 Debian 10+
- 构建工具：`build-essential`
- 包管理工具：`debhelper`, `dh-python`

### Fedora/RHEL/CentOS系统要求
- Fedora 30+ 或 RHEL/CentOS 8+
- 构建工具：`rpm-build`, `mock`
- 开发工具组：`Development Tools`

---

## Ubuntu/Debian系统打包

### 方法一：使用自动化脚本（推荐）

```bash
# 1. 进入项目根目录
cd /home/sen/dev/cpp/QGIS

# 2. 执行自动打包脚本
./scripts/build_debian_package.sh
```

**脚本功能：**
- 自动检查项目目录结构
- 从SVN更新debian配置文件
- 生成版本信息
- 使用dpkg-buildpackage构建包

### 方法二：手动构建

#### 1. 安装构建依赖

```bash
# 安装基础构建工具
sudo apt-get update
sudo apt-get install -y debhelper dh-python python3-dev pkgconf git

# 安装QGIS特定依赖（基于debian/control文件）
sudo apt-get build-dep qgis

# 或手动安装主要依赖
sudo apt-get install -y \
    cmake \
    libqt5dev \
    qttools5-dev \
    python3-dev \
    libgdal-dev \
    libgeos-dev \
    libproj-dev \
    libspatialite-dev \
    libspatialindex-dev \
    libqca-qt5-2-dev
```

#### 2. 构建包

```bash
# 使用dpkg-buildpackage构建
dpkg-buildpackage -us -uc -b

# 或使用debuild（如果已安装）
debuild -us -uc -b
```

### 生成的包文件

构建完成后，会在上级目录生成以下deb包：

- `qgis_*.deb` - 主程序包
- `qgis-common_*.deb` - 公共数据文件
- `libqgis-core*.deb` - 核心库
- `libqgis-gui*.deb` - GUI库
- `python3-qgis*.deb` - Python绑定
- `qgis-providers*.deb` - 数据提供者插件
- `qgis-server*.deb` - 服务器组件

### 安装包

```bash
# 安装所有生成的包
sudo dpkg -i ../*.deb

# 修复可能的依赖问题
sudo apt-get install -f
```

---

## Fedora/RHEL/CentOS系统打包

### 方法一：使用自动化脚本（推荐）

```bash
# 1. 进入rpm目录
cd rpm/

# 2. 安装构建依赖
sudo dnf install rpm-build rpmdevtools mock git

# 3. 构建完整RPM包
./buildrpms.sh

# 其他选项：
./buildrpms.sh -c    # 仅生成spec文件
./buildrpms.sh -s    # 仅构建SRPM
./buildrpms.sh -u    # 构建不稳定版本（包含git commit ID）
./buildrpms.sh -b    # 重新构建上次的SRPM
```

### 方法二：手动构建

#### 1. 安装构建依赖

```bash
# Fedora系统
sudo dnf install rpm-build rpmdevtools mock git
sudo dnf groupinstall "Development Tools"

# RHEL/CentOS系统
sudo yum install rpm-build rpmdevtools mock git
sudo yum groupinstall "Development Tools"
```

#### 2. 设置构建环境

```bash
# 设置RPM构建树
rpmdev-setuptree

# 将用户添加到mock组
sudo usermod -a -G mock $USER
```

#### 3. 生成spec文件并构建

```bash
cd rpm/

# 生成spec文件
./buildrpms.sh -c

# 手动构建（如果不使用mock）
rpmbuild -ba qgis.spec
```

### 配置文件说明

#### rpm/default.cfg
```bash
# 默认配置示例
OUTDIR="$HOME/rpmbuild"
ARCHS=("fedora-35-x86_64")
NOSIGN=true
```

#### rpm/local.cfg (可选)
```bash
# 本地配置覆盖
OUTDIR="/custom/build/path"
ARCHS=("fedora-35-x86_64" "fedora-35-i386")
NOSIGN=false
```

### 生成的包文件

构建完成后，会生成以下rpm包：

- `qgis-*.rpm` - 主程序包
- `qgis-devel-*.rpm` - 开发文件
- `qgis-grass-*.rpm` - GRASS集成插件
- `qgis-server-*.rpm` - 服务器组件
- `python3-qgis-*.rpm` - Python绑定

### 安装包

```bash
# 安装主包和依赖
sudo dnf install qgis-*.rpm

# 或使用rpm命令
sudo rpm -ivh qgis-*.rpm
```

---

## 高级配置

### 版本控制

项目版本在`CMakeLists.txt`中定义：

```cmake
set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "99")
set(CPACK_PACKAGE_VERSION_PATCH "0")
```

### 自定义构建选项

在构建前可以修改CMake选项：

```bash
# 示例：禁用某些功能
cmake -DWITH_3D=OFF -DWITH_GRASS=OFF ..
```

### 依赖管理

#### Debian系统依赖信息
位置：`debian/control`
- 构建依赖：`Build-Depends`字段
- 运行依赖：各包的`Depends`字段

#### RPM系统依赖信息
位置：`rpm/qgis.spec.template`
- 构建依赖：`BuildRequires`字段  
- 运行依赖：`Requires`字段

---

## 离线安装解决方案

### RPM包离线安装

#### 方法1：下载所有依赖包

```bash
# 在有网络的构建机器上执行
# 1. 创建离线安装包目录
mkdir -p qgis-offline-rpm

# 2. 复制构建好的QGIS RPM包
cp ~/rpmbuild/RPMS/x86_64/qgis-*.rpm qgis-offline-rpm/

# 3. 下载所有依赖包
cd qgis-offline-rpm/
sudo dnf download --resolve --alldeps --skip-broken qgis-*.rpm

# 或使用repoquery获取依赖列表
sudo dnf repoquery --requires --resolve qgis | xargs sudo dnf download --destdir .

# 4. 创建本地仓库
sudo dnf install createrepo_c
createrepo_c .

# 5. 打包传输
cd ..
tar -czf qgis-offline-complete.tar.gz qgis-offline-rpm/
```

#### 方法2：使用mock创建自包含构建

```bash
# 修改rpm/buildrpms.sh，添加依赖收集
cd rpm/

# 创建自定义配置
cat > local.cfg << 'EOF'
OUTDIR="$HOME/rpmbuild-offline"
ARCHS=("fedora-35-x86_64")
NOSIGN=true
COLLECT_DEPS=true
EOF

# 执行构建并收集依赖
./buildrpms.sh
```

#### 离线环境安装

```bash
# 1. 在离线机器上解压
tar -xzf qgis-offline-complete.tar.gz

# 2. 配置本地仓库
cat > /etc/yum.repos.d/qgis-local.repo << 'EOF'
[qgis-local]
name=QGIS Local Repository
baseurl=file:///path/to/qgis-offline-rpm
enabled=1
gpgcheck=0
EOF

# 3. 安装
sudo dnf install qgis --repo qgis-local

# 或直接rpm安装（可能缺少依赖）
sudo rpm -ivh qgis-offline-rpm/*.rpm
```

### DEB包离线安装

```bash
# 在线环境准备
mkdir -p qgis-offline-deb
cd qgis-offline-deb

# 下载依赖包
apt-get download $(apt-cache depends qgis | grep "Depends:" | cut -d: -f2)

# 下载QGIS包
cp ../*.deb .

# 创建Packages文件
dpkg-scanpackages . /dev/null | gzip -9c > Packages.gz

# 离线安装
sudo dpkg -i *.deb
sudo apt-get install -f  # 修复可能的依赖问题
```

---

## 常见问题与解决方案

### 构建错误

#### 1. 缺少依赖包
**症状：** 构建时报告缺少某个库或头文件
**解决：** 
```bash
# Debian/Ubuntu
sudo apt-get build-dep qgis

# Fedora
sudo dnf builddep qgis.spec
```

#### 2. 内存不足
**症状：** 编译过程中系统卡死或进程被杀死
**解决：**
```bash
# 限制并行编译进程数
export MAKEFLAGS="-j2"  # 使用2个并行进程

# 或在debian/rules中修改
override_dh_auto_build:
	dh_auto_build -- -j2
```

#### 3. Mock权限问题
**症状：** mock命令报告权限错误
**解决：**
```bash
sudo usermod -a -G mock $USER
# 重新登录或执行
newgrp mock
```

### 版本冲突

#### 包版本冲突
```bash
# 强制覆盖安装（谨慎使用）
sudo dpkg -i --force-overwrite *.deb

# 或先删除冲突的包
sudo apt-get remove qgis qgis-common
```

### 调试构建问题

#### 启用详细日志
```bash
# Debian构建
dpkg-buildpackage -us -uc -b -v

# RPM构建
_BUILDRPMS_DEBUG=1 ./buildrpms.sh
```

---

## 包发布与分发

### 创建本地APT仓库（Debian/Ubuntu）

```bash
# 1. 创建仓库目录
mkdir -p ~/myrepo

# 2. 复制deb文件
cp *.deb ~/myrepo/

# 3. 生成Packages文件
cd ~/myrepo
dpkg-scanpackages . /dev/null | gzip -9c > Packages.gz

# 4. 在/etc/apt/sources.list中添加：
# deb file:///home/user/myrepo ./
```

### 创建本地YUM仓库（Fedora/RHEL）

```bash
# 1. 安装createrepo
sudo dnf install createrepo

# 2. 创建仓库目录
mkdir -p ~/myrepo

# 3. 复制rpm文件
cp *.rpm ~/myrepo/

# 4. 创建仓库元数据
createrepo ~/myrepo

# 5. 在/etc/yum.repos.d/local.repo中添加：
# [local]
# name=Local Repository
# baseurl=file:///home/user/myrepo
# enabled=1
# gpgcheck=0
```

---

## 最佳实践

1. **使用自动化脚本**：优先使用项目提供的构建脚本，避免手动配置错误

2. **版本管理**：
   - 生产环境使用稳定版本标签
   - 测试环境可以使用`-u`参数构建开发版本

3. **构建环境**：
   - 使用干净的构建环境（Docker/VM）
   - 定期清理构建缓存

4. **依赖管理**：
   - 及时更新系统依赖包
   - 使用包管理器的build-dep功能

5. **测试验证**：
   - 构建完成后进行基本功能测试
   - 验证所有模块是否正常加载

---

## 参考资源

- **QGIS官方文档**: https://docs.qgis.org/
- **Debian打包指南**: https://www.debian.org/doc/manuals/maint-guide/
- **RPM打包指南**: https://rpm-packaging-guide.github.io/
- **项目源码**: https://github.com/qgis/QGIS
- **问题反馈**: https://github.com/qgis/QGIS/issues

---

## 更新日志

- **2025-08-28**: 创建初始版本，基于QGIS 3.99.0分析
- 包含完整的Debian和RPM打包流程
- 添加常见问题解决方案
- 提供最佳实践建议