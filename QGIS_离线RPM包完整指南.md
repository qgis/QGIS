# QGIS离线RPM包完整构建安装指南

## 概述

本指南详细介绍如何在有网络的环境中构建QGIS RPM包，并创建包含所有依赖的离线安装包，最后在离线环境中完成安装。

## 环境要求

### 构建环境（需要网络连接）
- **操作系统**: Fedora 35+, RHEL 8+, CentOS Stream 8+, Rocky Linux 8+
- **最低配置**: 4GB RAM, 20GB磁盘空间
- **推荐配置**: 8GB RAM, 50GB磁盘空间
- **网络**: 稳定的互联网连接

### 目标离线环境
- **操作系统**: 与构建环境相同或兼容的Linux发行版
- **架构**: x86_64（默认）
- **磁盘空间**: 至少5GB用于安装包

---

## 第一阶段：构建环境准备

### 1.1 安装构建工具

```bash
# Fedora系统
sudo dnf groupinstall "Development Tools" "RPM Development Tools"
sudo dnf install rpm-build rpmdevtools mock git createrepo_c

# RHEL/CentOS/Rocky系统  
sudo yum groupinstall "Development Tools"
sudo yum install rpm-build rpmdevtools mock git createrepo
# 如果没有createrepo，可以安装createrepo_c
sudo dnf install epel-release
sudo dnf install createrepo_c
```

### 1.2 配置构建环境

```bash
# 设置RPM构建目录
rpmdev-setuptree

# 将当前用户添加到mock组
sudo usermod -a -G mock $USER

# 重新登录或刷新组权限
newgrp mock

# 验证mock权限
mock --version
```

### 1.3 获取QGIS源码

```bash
# 克隆QGIS源码仓库
git clone https://github.com/qgis/QGIS.git
cd QGIS

# 切换到稳定版本（可选）
git checkout lts-3_34  # 或其他稳定版本

# 确认版本信息
grep "SET(CPACK_PACKAGE_VERSION" CMakeLists.txt
```

---

## 第二阶段：构建QGIS RPM包

### 2.1 准备构建配置

```bash
# 进入rpm目录
cd rpm/

# 检查默认配置
cat default.cfg

# 创建本地配置（可选）
cat > local.cfg << 'EOF'
# 构建输出目录（默认为result，即rpm/result/）
OUTDIR="result"

# 目标架构（可多选）
ARCHS=("fedora-39-x86_64")  # 根据实际系统调整

# 是否签名包（离线环境通常不需要）
NOSIGN=true
EOF
```

### 2.2 检查构建依赖

```bash
# 安装QGIS构建依赖
sudo dnf builddep ../CMakeLists.txt

# 手动安装可能缺失的依赖
sudo dnf install \
    qt5-qtbase-devel qt5-qttools-devel qt5-qtsvg-devel \
    qt5-qtwebkit-devel qt5-qtmultimedia-devel qt5-qtpositioning-devel \
    qt5-qtsensors-devel qt5-qtserialport-devel qt5-qt3d-devel \
    gdal-devel geos-devel proj-devel sqlite-devel \
    spatialite-devel spatialindex-devel \
    python3-devel python3-qscintilla-qt5-devel \
    python3-pyqt5-devel python3-sip-devel \
    qca-qt5-devel qscintilla-qt5-devel \
    qtkeychain-qt5-devel qwt-qt5-devel \
    netcdf-devel hdf5-devel postgresql-devel \
    fcgi-devel expat-devel libzip-devel \
    protobuf-devel draco-devel \
    flex bison cmake ninja-build
```

### 2.3 执行构建

```bash
# 生成spec文件（检查配置）
./buildrpms.sh -c

# 检查生成的spec文件
less qgis.spec

# 执行完整构建
./buildrpms.sh

# 或分步构建（便于调试）
./buildrpms.sh -s    # 先构建SRPM
./buildrpms.sh -b    # 再构建二进制RPM
```

### 2.4 验证构建结果

```bash
# 检查构建输出目录
ls -la rpm/result/

# 查看生成的RPM包
find rpm/result -name "*.rpm" -type f

# 检查包信息
rpm -qip rpm/result/fedora-39-x86_64/qgis-*.rpm
```

---

## 第三阶段：创建离线安装包

### 3.1 使用自动化脚本收集依赖

项目已提供现成的依赖收集脚本，无需手动创建：

```bash
# 进入scripts目录
cd ../scripts/

# 运行依赖收集脚本
./collect_offline_rpms.sh
```

该脚本会自动：
- 智能查找构建好的QGIS RPM包
- 下载所有运行时依赖包  
- 创建本地RPM仓库元数据
- 生成压缩包便于传输

### 3.2 复制安装脚本到离线包

```bash
# 复制安装相关脚本到离线包目录
OFFLINE_DIR=$(ls -d qgis-offline-* | head -1)
cp install-qgis.sh uninstall-qgis.sh README-offline.md "$OFFLINE_DIR/"

# 创建最终传输包
tar -czf "${OFFLINE_DIR}.tar.gz" "$OFFLINE_DIR"
```

### 3.3 验证离线包完整性

```bash
# 进入创建的离线包目录
cd "$OFFLINE_DIR"

# 检查RPM包完整性
rpm -qp *.rpm 2>/dev/null | head -20

# 查看依赖关系
rpm -qpR qgis-*.rpm | head -10

# 验证仓库元数据
ls -la repodata/

# 检查脚本文件
ls -la *.sh README-offline.md
```

---

## 第四阶段：传输到离线环境

### 4.1 打包和传输

```bash
# 传输方式选择

# 方法A: SCP传输
scp qgis-offline-*.tar.gz user@offline-server:/tmp/

# 方法B: USB拷贝
cp qgis-offline-*.tar.gz /media/usb/

# 方法C: 网络共享
# 放置在共享目录供离线机器下载
```

### 4.2 离线环境解压

```bash
# 在离线机器上
cd /tmp  # 或其他目录

# 解压安装包
tar -xzf qgis-offline-*.tar.gz

# 进入目录
cd qgis-offline-*/

# 查看内容
ls -la
cat README-offline.md
```

---

## 第五阶段：离线环境安装

### 5.1 使用自动安装脚本

```bash
# 运行自动安装脚本（推荐）
sudo ./install-qgis.sh
```

安装脚本会自动：
- 配置本地RPM仓库
- 安装QGIS和依赖包
- 验证安装结果

### 5.2 手动安装（高级用户）

如果自动脚本失败，可以手动安装：

```bash
# 1. 手动配置仓库
sudo tee /etc/yum.repos.d/qgis-offline.repo > /dev/null << 'EOF'
[qgis-offline]
name=QGIS Offline Repository
baseurl=file:///tmp/qgis-offline-20250828
enabled=1
gpgcheck=0
priority=1
EOF

# 2. 清理缓存
sudo dnf clean all
sudo dnf makecache --repo=qgis-offline

# 3. 安装QGIS
sudo dnf install qgis --repo=qgis-offline -y

# 4. 或强制安装所有包（最后手段）
sudo rpm -ivh *.rpm --force --nodeps
```

### 5.3 验证安装

```bash
# 检查安装状态
rpm -qa | grep qgis

# 测试命令行
qgis --version

# 测试图形界面（需要X11环境）
qgis &

# 检查Python绑定
python3 -c "from qgis.core import *; print('QGIS Python绑定正常')"
```

---

## 故障排除

### 构建阶段问题

#### 1. 构建依赖缺失
```bash
# 查看具体缺失的依赖
./buildrpms.sh 2>&1 | grep -i "not found\|missing"

# 手动安装缺失依赖
sudo dnf install [缺失的包名]
```

#### 2. Mock权限问题
```bash
# 重新配置mock用户组
sudo usermod -a -G mock $USER
sudo newgrp mock

# 测试mock权限
mock --version
```

#### 3. 磁盘空间不足
```bash
# 清理构建缓存
rm -rf rpm/result/*/build
mock --clean

# 检查磁盘空间
df -h
```

### 依赖收集问题

#### 1. 网络连接问题
```bash
# 测试网络连接
ping -c 3 download.fedoraproject.org

# 更新仓库缓存
sudo dnf makecache --refresh
```

#### 2. 依赖解析失败
```bash
# 手动下载核心依赖
sudo dnf download gdal-libs geos proj sqlite

# 检查依赖关系
rpm -qpR qgis-*.rpm | sort | uniq
```

### 安装阶段问题

#### 1. 仓库配置错误
```bash
# 检查仓库路径
cat /etc/yum.repos.d/qgis-offline.repo

# 修正baseurl路径
sudo sed -i 's|baseurl=.*|baseurl=file:///正确的路径|' /etc/yum.repos.d/qgis-offline.repo
```

#### 2. RPM包损坏
```bash
# 检查RPM包完整性
rpm -qp --checksig *.rpm

# 重新下载损坏的包
# （需要在有网络的机器上重新收集）
```

#### 3. 依赖冲突
```bash
# 查看冲突详情
sudo dnf install qgis --repo=qgis-offline --verbose

# 强制安装（不推荐，仅紧急情况）
sudo rpm -ivh qgis-*.rpm --force --replacefiles
```

---

## 高级配置

### 多架构支持

如果需要支持32位系统：

```bash
# 修改构建配置
cat > rpm/local.cfg << 'EOF'
ARCHS=("fedora-39-x86_64" "fedora-39-i386")
EOF

# 分别为每个架构创建离线包
```

### 自定义构建选项

```bash
# 修改spec文件，添加自定义选项
cp rpm/qgis.spec.template rpm/qgis.spec.custom

# 编辑自定义选项
vim rpm/qgis.spec.custom

# 使用自定义spec文件构建
mock --buildsrpm --spec qgis.spec.custom --sources ./sources
```

### 企业环境部署

```bash
# 创建企业内部仓库
# 1. 搭建内部HTTP服务器
sudo dnf install httpd
sudo systemctl start httpd

# 2. 复制RPM包到web目录
sudo cp -r qgis-offline-*/* /var/www/html/qgis/

# 3. 创建仓库元数据
sudo createrepo_c /var/www/html/qgis/

# 4. 客户端配置
cat > /etc/yum.repos.d/qgis-internal.repo << 'EOF'
[qgis-internal]
name=Internal QGIS Repository
baseurl=http://internal-server/qgis/
enabled=1
gpgcheck=0
EOF
```

---

## 最佳实践

### 1. 版本管理
- 为每个QGIS版本创建独立的离线包
- 保留构建日志和配置文件
- 使用语义化版本命名

### 2. 测试验证  
- 在隔离环境中测试离线安装
- 验证所有核心功能正常工作
- 测试Python插件加载

### 3. 文档维护
- 记录每次构建的配置变更
- 维护已知问题和解决方案清单
- 定期更新依赖包列表

### 4. 安全考虑
- 验证RPM包的数字签名
- 在受信任环境中进行构建
- 定期更新安全补丁

---

## 完整工作流程总结

### 构建环境操作
```bash
# 1. 准备环境
git clone https://github.com/qgis/QGIS.git
cd QGIS
sudo dnf install rpm-build mock createrepo_c

# 2. 构建RPM包
cd rpm/
./buildrpms.sh

# 3. 创建离线包
cd ../scripts/
./collect_offline_rpms.sh
cp install-qgis.sh uninstall-qgis.sh README-offline.md qgis-offline-*/
tar -czf qgis-offline.tar.gz qgis-offline-*/
```

### 离线环境操作
```bash
# 1. 传输并解压
tar -xzf qgis-offline.tar.gz
cd qgis-offline-*/

# 2. 安装
sudo ./install-qgis.sh

# 3. 验证
qgis --version
```

---

## 总结

本指南提供了完整的QGIS离线RPM包构建和部署流程：

1. **构建阶段**: 设置环境、编译源码、生成RPM包
2. **打包阶段**: 收集依赖、创建离线仓库、生成安装脚本
3. **传输阶段**: 打包压缩、传输到目标环境
4. **安装阶段**: 配置仓库、安装软件、验证功能

通过遵循本指南并使用提供的自动化脚本，你可以在完全离线的环境中成功部署QGIS，满足企业内网、科研环境等特殊场景的需求。

---

**文档版本**: 2.0  
**最后更新**: 2025-08-28  
**适用版本**: QGIS 3.x, Fedora/RHEL/CentOS 8+